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

#ifndef XPC_H_
#define XPC_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define XPC_CONNECTION_MACH_SERVICE_LISTENER (1 << 0)

#ifndef HAVE_libdispatch
typedef void *dispatch_queue_t;
#endif

typedef struct DBusConnection DBusConnection;

struct xpc_connection {
	bool listener;
	DBusConnection *conn;
	char busname[255];
};

typedef struct xpc_connection *xpc_connection_t;

xpc_connection_t xpc_connection_create_mach_service(const char *name,
	dispatch_queue_t targetq, uint64_t flags);

#endif /* XPC_H_ */
