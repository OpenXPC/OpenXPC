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

#include <stdio.h>

#include "xpc2/log.h"

int log_oom()
{
	fprintf(stderr, "Out of memory.\n");
	return -ENOMEM;
}
