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

#include "launchd.hh"

int Job::exec()
{
	int i = fork();
	if (i < -1)
		return log_job_errno(job, kError, errno,
			"Failed to create subprocess: %m");
}
