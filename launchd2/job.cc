/*
 *		PROPRIETARY NOTICE
 *
 *  This source code is unpublished proprietary information
 *  constituting, or derived under license from LaunchD-Reloaded(tm).
 *
 *
 *		Copyright Notice
 *
 *  Notice of copyright on this source code product does not indicate
 *  publication.
 *
 *	(c) 2021 The Project Maintainers of LaunchD-Reloaded.
 *		All rights reserved.
 */

#include <sys/event.h>

#include <limits.h>
#include <unistd.h>
#include <cassert>

#include "xpc2/procutil.h"
#include "xpc2/util.h"
#include "launchd.hh"

void Job::event_cb(int ident, int filter, int fflags, int data)
{
	log_error("Got event %d/%d\n", ident, filter);
}

int Job::exec()
{
	int i;
	int waitfd[2];

	pipe(waitfd);
	i = fork();

	if (i < -1)
		return log_job_errno(job, kError, errno,
			"Failed to create subprocess: %m");
	else if (i == 0) { /* subprocess */
		char dispose;
		read(waitfd[0], &dispose, 1);
		close(waitfd[0]);
		setup_subproc(-1);
	} else { /* parent process */
		struct kevent kev;

		EV_SET(&kev, i, EVFILT_PROC, EV_ADD, NOTE_EXIT | NOTE_FORK, 0,
			this);
		if (kevent(manager.kq, &kev, 1, NULL, 0, NULL) == -1)
			return log_error_errno(errno,
				"Failed to attach PROC filter to subprocess: %m");

		close(waitfd[0]);

		if (write(waitfd[1], "0", 1) <= 0) /* let subprocess begin */
			log_error_errno(errno,
				"Failed to write to waitfd: %m\n");
		close(waitfd[1]);

		log_job(this, kInfo, "Launched job");
	}

	return 0;
}

int Job::setup_subproc(int waitfd)
{
	int r;
	int fd = -1;
	const char *exe;
	const char **argv;
	int argc;

	if (is_bootstrap)
		fd = 55;

	close_fds_except(&fd, 1);

	exe = program.has_value() ? program->c_str() : progargs[0].c_str();
	if (progargs.empty()) {
		assert(program.has_value());
		argc = 1;
		argv = (const char **)malloc(sizeof *argv);
		assert(argv != NULL);
		argv[1] = program->c_str();
	} else {
		argc = progargs.size();
		argv = (const char **)malloc((sizeof *argv) * argc);
		assert(argv != NULL);
		for (int i = 0; i < argc; i++)
			argv[i] = progargs[i].c_str();
	}

	r = execvpe(exe, (char *const *)argv, NULL);

	log_error_errno(errno, "Failed to launch program <%s>: %m", exe);
	exit(-errno);
}
