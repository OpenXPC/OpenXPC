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
 
#include <stdlib.h>
#include <stdio.h>

#include <dbus/dbus.h>

#include "xpc2/xpc.h"
#include "xpc2/log.h"

static DBusConnection *conn_bus = NULL;

static int setup_xpc()
{
	DBusError err;
	int r;

	dbus_error_init(&err);

	if (conn_bus)
		return 0;

	conn_bus = dbus_bus_get(DBUS_BUS_SESSION, &err);
	if (dbus_error_is_set(&err)) {
		log_error("Connection Error (%s)\n", err.message);
		dbus_error_free(&err);
		return -1;
	}
	if (NULL == conn_bus) {
		exit(1);
	}

	return 0;
}

xpc_connection_t xpc_connection_create_mach_service(const char *name,
	dispatch_queue_t targetq, uint64_t flags)
{
	xpc_connection_t res = malloc(sizeof *res);
	DBusError err;
	int r;

	if (!res)
		return log_oom_null();

	setup_xpc();
	dbus_error_init(&err);

	if (flags & XPC_CONNECTION_MACH_SERVICE_LISTENER) {
		res->listener = true;

		r = dbus_bus_request_name(conn_bus, name,
			DBUS_NAME_FLAG_REPLACE_EXISTING, &err);
		if (dbus_error_is_set(&err)) {
			log_error("Name Error (%s)\n", err.message);
			dbus_error_free(&err);
		}
		if (DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER != r) {
			exit(1);
		}
	}

	return res;
}
