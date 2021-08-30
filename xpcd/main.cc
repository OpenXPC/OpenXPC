/*
 *		PROPRIETARY NOTICE
 *
 *  This source code is unpublished proprietary information
 *  constituting, or derived under license from OpenXPC(tm).
 *
 *
 *		Copyright Notice
 *
 *  Notice of copyright on this source code product does not indicate
 *  publication.
 *
 *	(c) 2021 The Project Maintainers of OpenXPC.
 *		All rights reserved.
 */

#include <sys/event.h>
#include <sys/signal.h>

#include <cassert>

#include "OpenXPC/config.h"
#include "OpenXPC/log.h"

#include "launchd.hh"

Manager manager;

/* let kqueue handle a signal */
static int
kqueue_signal(int kq, int signo)
{
	struct kevent kev;

	EV_SET(&kev, signo, EVFILT_SIGNAL, EV_ADD, 0, 0, &manager);

	if (kevent(kq, &kev, 1, 0, 0, 0) == -1) {
		log_error("Failed to watch signal with kqueue");
		exit(EXIT_FAILURE);
	}

	/**
	 * On BSD, setting a signal event filter on a Kernel Queue does NOT
	 * supersede ordinary signal disposition. Therefore we ignore the
	 * signal; it'll be multiplexed into our event loop instead.
	 */
	assert(signal(signo, SIG_IGN) != SIG_ERR);

	return 0;
}

int
Domain::bootstrap_sys()
{
	Job *bsjob;

	jobs.emplace_back(std::make_unique<Job>());
	bsjob = jobs.back().get();

	bsjob->label = "com.apple.launchctl.bootstrap-sys";
	bsjob->progargs = { LAUNCHCTL_PATH, "/etc" };

	bsjob->exec();

	return 0;
}

void
Manager::event_cb(int ident, int filter, int fflags, int data)
{
	if (filter == EVFILT_SIGNAL) {
		log_info("Received signal %d", ident);
		if (ident == SIGINT || ident == SIGTERM)
			should_exit = true;
	}
}

int
Manager::main(int argc, char *argv[])
{
	struct kevent kev[5];
	int nev;

	log_info(LAUNCHD2_STRING ": " FULL_GATE_STRING);

	if ((kq = kqueue()) == -1)
		return -log_error_errno(errno,
			"Failed to create Kernel Queue: %m");

	kqueue_signal(kq, SIGINT);

	dom_sys = std::make_unique<Domain>();
	dom_sys->type = Domain::kSystem;
	dom_sys->bootstrap_sys();

	while (!should_exit) {
		/* the main loop */
		nev = kevent(kq, NULL, 0, kev, 5, NULL);

		if (nev == -1) {
			log_error("kevent failed: %m");
			exit(EXIT_FAILURE);
		} else if (nev == 0) {
			if (errno == EINTR)
				continue;
			log_error("Unexpected timeout");
		} else if (nev > 0) {
			for (int i = 0; i < nev; i++) {
				KEventHandler *handler =
					(KEventHandler *)kev[i].udata;

				handler->event_cb(kev[i].ident, kev[i].filter,
					kev[i].fflags, kev[i].data);
			}
		}
	}

	log_info("Bootstrap Service shutting down now");

	return 0;
}

int
main(int argc, char *argv[])
{
	return manager.main(argc, argv);
}
