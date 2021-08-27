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

#ifndef LOG_H_
#define LOG_H_

#include <errno.h>

int log_oom();

#define log_error(...) fprintf(stderr, __VA_ARGS__)
#define log_oom_null() (log_oom(), NULL)

#endif /* LOG_H_ */
