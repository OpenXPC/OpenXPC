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

#include "xpc2/config.h"
#include "xpc2/log.h"

#include "launchd.hh"

Manager manager;

int Domain::bootstrap_sys()
{
	Job *bsjob;

	jobs.emplace_back(std::make_unique<Job>());
	bsjob = jobs.back().get();

	bsjob->name = "com.apple.launchd.bootstrap-sys";
	bsjob->progargs = { LAUNCHCTL_PATH, "/etc" };

	bsjob->exec();

	return 0;
}

int Manager::main(int argc, char *argv[])
{
	log_info(LAUNCHD2_STRING);
	dom_sys = std::make_unique<Domain>();
	dom_sys->type = Domain::kSystem;
	dom_sys->bootstrap_sys();
	return 0;
}

int main(int argc, char *argv[])
{
	return manager.main(argc, argv);
}
