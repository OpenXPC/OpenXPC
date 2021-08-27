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

#include <list>
#include <memory>
#include <string>
#include <vector>

#include "xpc2/log.h"

#define log_job(job, level, ...)                                               \
	ldr_log(level, __FILE__, __LINE__, __func__, __VA_ARGS__)

#define log_job_errno(job, level, error, ...)                                  \
	(log_job(job, level, __VA_ARGS__), -error)

struct Domain;

struct Job {
	std::string name;
	std::string plist_path;
	std::vector<std::string> progargs;
	Domain *domain;

	/* launch the job */
	int exec();
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
};

class Manager {
	std::unique_ptr<Domain> dom_sys;

public:
	int main(int argc, char *argv[]);
};

extern Manager manager;

#endif /* LAUNCHD_H_ */
