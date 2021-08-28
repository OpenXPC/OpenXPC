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

#ifndef LAUNCHD_H_
#define LAUNCHD_H_

#include <sys/types.h>
#include <sys/socket.h>

#include <list>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "xpc2/log.h"

#define log_job(job, level, ...)                                               \
	ldr_log(level, __FILE__, __LINE__, __func__, __VA_ARGS__)

#define log_job_errno(job, level, error, ...)                                  \
	(log_job(job, level, __VA_ARGS__), -error)

struct Domain;
class Socket;
class Job;

/* Any object subscribing to KEvents implements this interface. */
struct KEventHandler {
	virtual void event_cb(int ident, int filter, int fflags, int data)
	{
		log_error("Unhandled KEvent");
	}
};

/* A group of 'equivalent' sockets (e.g. one IPv4, one IPv6).  */
class SocketGroup : KEventHandler {
	Job *job;

public:
	std::list<int> fds;

	/* from plist */
	std::string label;
};

class Job : KEventHandler {
public:
	Domain *domain = NULL;
	enum { kOffline, kOnline, kRunning } state = kOffline;

	std::string plist_path;
	bool is_bootstrap: 1; /* passed private IPC channel */

	/* plist-derived stuff */
	std::string label;
	std::optional<std::string> program = std::nullopt;
	std::vector<std::string> progargs;
	std::vector<SocketGroup> sockgroups;

	int exec(); /* launch the job */

private:
	/**
	 * Read-end of waitfd - if setup_subproc fails in subproc, it writes
	 * error here.
	 */
	int wait_fd = -1;

	void event_cb(int ident, int filter, int fflags, int data);

	/* setup in newly-created subprocess and run the program specified */
	int setup_subproc(int waitfd);
};

struct Domain {
	Domain *dom_parent; /* or NULL */
	enum {
		kSystem,
		kUser,
	} type;

	uid_t uid; /* if type = kUser, what UID? */

	std::list<std::unique_ptr<Domain> > subdomains;
	std::list<std::unique_ptr<Job> > jobs;

	/* Start the plist-loading job. */
	int bootstrap_sys();

private:
};

class Manager : KEventHandler {
	bool should_exit = false;
	std::unique_ptr<Domain> dom_sys;

	void event_cb(int ident, int filter, int fflags, int data);

public:
	int kq;

	int main(int argc, char *argv[]);
};

extern Manager manager;

#endif /* LAUNCHD_H_ */
