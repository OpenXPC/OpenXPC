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

#include "xpc2/xpc.h"

int main()
{
	xpc_connection_create_mach_service("com.apple.launchd", NULL,
		XPC_CONNECTION_MACH_SERVICE_LISTENER);
	return 0;
}
