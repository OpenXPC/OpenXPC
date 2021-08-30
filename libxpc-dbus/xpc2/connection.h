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

#ifndef CONNECTION_H_
#define CONNECTION_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "xpc2/config.h"

#ifdef HAVE_dispatch
#include <dispatch/dispatch.h>
#else
typedef void *dispatch_queue_t;
#endif

#define XPC_CONNECTION_MACH_SERVICE_LISTENER (1 << 0)

xpc_connection_t xpc_connection_create_mach_service(const char *name,
	dispatch_queue_t targetq, uint64_t flags);
void xpc_connection_send_message(xpc_connection_t xconn, xpc_object_t xmsg);

#endif /* CONNECTION_H_ */
