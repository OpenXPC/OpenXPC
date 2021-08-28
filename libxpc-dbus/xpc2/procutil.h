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

#ifndef PROCUTIL_H_
#define PROCUTIL_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Close all FDs of number 3 or above not in the array fds[nfds] */
int close_fds_except(int fds[], int nfds);

#ifdef __cplusplus
}
#endif

#endif /* PROCUTIL_H_ */
