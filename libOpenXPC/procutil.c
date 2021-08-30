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

#include <stdbool.h>
#include <unistd.h>

#include "OpenXPC/procutil.h"

int
close_fds_except(int fds[], int nfds)
{
	for (int i = 3; i < 1023; i++) {
		bool present = false;

		for (int j = 0; j < nfds; j++)
			if (fds[j] == i) {
				present = true;
				break;
			}

		if (!present)
			close(i);
	}

	return 0;
}
