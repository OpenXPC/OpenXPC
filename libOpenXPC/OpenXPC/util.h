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

#ifndef UTIL_H_
#define UTIL_H_

#include <errno.h>
#include <unistd.h>

static inline int ldr_close_internal(int *fd)
{
	int r = close(*fd);
	*fd = -1;
	if (r < 0)
		return -errno;
	return 0;
}

#define ldr_close(fd) ldr_close_internal(&fd)

#endif /* UTIL_H_ */
