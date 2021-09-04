#include <assert.h>
#include <stddef.h>

#include <dbus/dbus-protocol.h>
#include <dbus/dbus.h>

#include "OpenXPC/log.h"
#include "OpenXPC/xpc.h"
#include "xpc_internal.h"

struct _xpc_connection_s {
	struct xpc_object base;

	const char *busname;
	DBusConnection *conn;
	bool passive; /* is it a listener */
};

/*
 * serialisation of XPC objects to D-Bus messages
 */

static const char *
xpc_dbus_type(struct xpc_object *xo)
{
	xpc_type_t type = xo->xo_xpc_type;

	return type == XPC_TYPE_BOOL	    ? DBUS_TYPE_BOOLEAN_AS_STRING :
		type == XPC_TYPE_INT64	    ? DBUS_TYPE_INT64_AS_STRING :
		type == XPC_TYPE_UINT64	    ? DBUS_TYPE_UINT64_AS_STRING :
		type == XPC_TYPE_DATE	    ? DBUS_TYPE_INT64_AS_STRING :
		type == XPC_TYPE_STRING	    ? DBUS_TYPE_STRING_AS_STRING :
		type == XPC_TYPE_DATA	    ? "ay" :
		type == XPC_TYPE_DICTIONARY ? "a{sv}" :
		type == XPC_TYPE_ARRAY	    ? "av" :
		type == XPC_TYPE_FD	    ? DBUS_TYPE_UNIX_FD_AS_STRING :
						    DBUS_TYPE_BOOLEAN_AS_STRING;
}

static int
append_xpc(struct xpc_object *xo, DBusMessageIter *iter)
{
	if (xo->xo_xpc_type == XPC_TYPE_BOOL) {
		dbus_bool_t val = xpc_bool_get_value(xo);
		dbus_message_iter_append_basic(iter, DBUS_TYPE_BOOLEAN, &val);
	} else if (xo->xo_xpc_type == XPC_TYPE_INT64) {
		int64_t val = xpc_int64_get_value(xo);
		dbus_message_iter_append_basic(iter, DBUS_TYPE_INT64, &val);
	} else if (xo->xo_xpc_type == XPC_TYPE_UINT64) {
		uint64_t val = xpc_uint64_get_value(xo);
		dbus_message_iter_append_basic(iter, DBUS_TYPE_UINT64, &val);
	} else if (xo->xo_xpc_type == XPC_TYPE_DATE) {
		int64_t val = xpc_date_get_value(xo);
		dbus_message_iter_append_basic(iter, DBUS_TYPE_INT64, &val);
	} else if (xo->xo_xpc_type == XPC_TYPE_STRING) {
		const char *val = xpc_string_get_string_ptr(xo);
		dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &val);
	} else if (xo->xo_xpc_type == XPC_TYPE_DATA)
		dbus_message_iter_append_fixed_array(iter, DBUS_TYPE_BYTE,
			xpc_data_get_bytes_ptr(xo), xpc_data_get_length(xo));
	else if (xo->xo_xpc_type == XPC_TYPE_FD) {
		int fd = xpc_ext_fd_get_fd(xo);
		dbus_message_iter_append_basic(iter, DBUS_TYPE_UNIX_FD, &fd);
	} else if (xo->xo_xpc_type == XPC_TYPE_DICTIONARY) {
		__block DBusMessageIter sub;
		bool succ;

		dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY, "{sv}",
			&sub);

		succ = xpc_dictionary_apply(xo,
			^bool(const char *key, xpc_object_t value) {
				DBusMessageIter sub2, sub3;

				dbus_message_iter_open_container(&sub,
					DBUS_TYPE_DICT_ENTRY, NULL, &sub2);
				dbus_message_iter_append_basic(&sub2,
					DBUS_TYPE_STRING, &key);
				dbus_message_iter_open_container(&sub2,
					DBUS_TYPE_VARIANT, xpc_dbus_type(value),
					&sub3);
				append_xpc(value, &sub3);
				if (!dbus_message_iter_close_container(&sub2,
					    &sub3) ||
					!dbus_message_iter_close_container(&sub,
						&sub2))
					return false;
				return true;
			});

		if (!succ || !dbus_message_iter_close_container(iter, &sub))
			goto oom;
	} else if (xo->xo_xpc_type == XPC_TYPE_ARRAY) {
		__block DBusMessageIter sub;
		bool succ;

		dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY,
			DBUS_TYPE_VARIANT_AS_STRING, &sub);

		succ = xpc_array_apply(xo,
			^bool(size_t idx, xpc_object_t value) {
				DBusMessageIter sub2, sub3;

				dbus_message_iter_open_container(&sub,
					DBUS_TYPE_VARIANT, xpc_dbus_type(value),
					&sub2);
				append_xpc(value, &sub2);
				if (!dbus_message_iter_close_container(&sub,
					    &sub2))
					return false;
				return true;
			});

		if (!succ || !dbus_message_iter_close_container(iter, &sub))
			goto oom;
	} else {
		dbus_bool_t val = false;
		dbus_message_iter_append_basic(iter, DBUS_TYPE_BOOLEAN, &val);
	}

	return 0;

oom:
	return log_oom();
}

static DBusConnection *conn_bus = NULL;

static int
setup_xpc()
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

xpc_connection_t
xpc_connection_create_mach_service(const char *name, dispatch_queue_t targetq,
	uint64_t flags)
{
	xpc_connection_t res = malloc(sizeof *res);
	DBusError err;
	int r;

	if (!res)
		return log_oom_null();

	res->base.xo_size = 0;
	res->base.xo_xpc_type = XPC_TYPE_CONNECTION;
	res->base.xo_flags = 0;
	res->base.xo_refcnt = 1;
#if MACH
	res->base.xo_audit_token = NULL;
#endif

	setup_xpc();
	dbus_error_init(&err);

	if (flags & XPC_CONNECTION_MACH_SERVICE_LISTENER) {
		res->passive = true;

		r = dbus_bus_request_name(conn_bus, name,
			DBUS_NAME_FLAG_REPLACE_EXISTING, &err);
		if (dbus_error_is_set(&err)) {
			log_error("Name Error (%s)\n", err.message);
			dbus_error_free(&err);
		}
		if (DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER != r) {
			log_error("Failure\n");
			exit(1);
		}
	}

	res->conn = conn_bus;
	res->busname = strdup(name);

	return res;
}

static xpc_object_t deserialise_message(DBusMessage *msg);

void
xpc_connection_send_message(xpc_connection_t xconn, xpc_object_t xmsg)
{
	DBusMessage *msg;
	DBusMessageIter iter;
	DBusError error;

	if (xconn->passive)
		return (void)log_error(
			"Can't send message to a passive connection");

	dbus_error_init(&error);

	msg = dbus_message_new_method_call(xconn->busname,
		"/org/LaunchD_Reloaded/xpc", "org.LaunchD_Reloaded.xpc",
		"xpcMessage");

	dbus_message_iter_init_append(msg, &iter);
	if (append_xpc(xmsg, &iter) < 0) {
		log_error("Failed to serialise message");
		dbus_message_unref(msg);
		return;
	}

	if (!dbus_connection_send_with_reply_and_block(xconn->conn, msg, -1,
		    &error))
		log_error("Failed to send message to the D-Bus: %s\n",
			error.message);

	deserialise_message(msg);

	dbus_error_free(&error);
}

static xpc_object_t deserialise_object(DBusMessageIter *iter);

static xpc_object_t
deserialise_arr(DBusMessageIter *iter)
{
	xpc_object_t arr = xpc_array_create(NULL, 0);

	while (dbus_message_iter_get_arg_type(iter) != DBUS_TYPE_INVALID) {
		DBusMessageIter sub;
		const char *key;

		assert(dbus_message_iter_get_arg_type(iter) ==
			DBUS_TYPE_VARIANT);
		dbus_message_iter_recurse(iter, &sub);

		xpc_array_append_value(arr, deserialise_object(&sub));

		dbus_message_iter_next(iter);
	}

	return arr;
}

static xpc_object_t
deserialise_dict(DBusMessageIter *iter)
{
	xpc_object_t dict = xpc_dictionary_create(NULL, NULL, 0);

	while (dbus_message_iter_get_arg_type(iter) != DBUS_TYPE_INVALID) {
		DBusMessageIter sub, sub2;
		const char *key;

		dbus_message_iter_recurse(iter, &sub);
		assert(dbus_message_iter_get_arg_type(&sub) ==
			DBUS_TYPE_STRING);
		dbus_message_iter_get_basic(&sub, &key);
		assert(key);

		dbus_message_iter_next(&sub);
		assert(dbus_message_iter_get_arg_type(&sub) ==
			DBUS_TYPE_VARIANT);
		dbus_message_iter_recurse(&sub, &sub2);

		xpc_dictionary_set_value(dict, key, deserialise_object(&sub2));

		dbus_message_iter_next(iter);
	}

	return dict;
}

static xpc_object_t
deserialise_object(DBusMessageIter *iter)
{
	char type = dbus_message_iter_get_arg_type(iter);
	switch (type) {
	case DBUS_TYPE_BOOLEAN: {
		dbus_bool_t val;
		dbus_message_iter_get_basic(iter, &val);
		return xpc_bool_create(val);
	}
	case DBUS_TYPE_INT64: {
		int64_t val;
		dbus_message_iter_get_basic(iter, &val);
		return xpc_int64_create(val);
	}
	case DBUS_TYPE_UINT64: {
		uint64_t val;
		dbus_message_iter_get_basic(iter, &val);
		return xpc_uint64_create(val);
	}
	case DBUS_TYPE_STRING: {
		const char *val;
		dbus_message_iter_get_basic(iter, &val);
		return xpc_string_create(val);
	}
	case DBUS_TYPE_ARRAY: {
		char arrtype = dbus_message_iter_get_element_type(iter);

		switch (arrtype) {
		case DBUS_TYPE_DICT_ENTRY: {
			DBusMessageIter sub;
			dbus_message_iter_recurse(iter, &sub);
			return deserialise_dict(&sub);
		}
		case DBUS_TYPE_VARIANT: {
			DBusMessageIter sub;
			dbus_message_iter_recurse(iter, &sub);
			return deserialise_arr(&sub);
		}
		case DBUS_TYPE_BYTE: {
			int len;
			unsigned char *val;
			dbus_message_iter_get_fixed_array(iter, val, &len);
			return xpc_data_create(val, len);
		}

		default:
			log_error("Bad array type %c\n", arrtype);
			return NULL;
		}
	}
	case DBUS_TYPE_UNIX_FD: {
		int val;
		dbus_message_iter_get_basic(iter, &val);
		return xpc_fd_create(val);
	}

	default:
		log_error("Invalid type %c\n", type);
		return NULL;
	}
}

/* Deserialise an xpcMessage message into a resultant XPC object. */
static xpc_object_t
deserialise_message(DBusMessage *msg)
{
	DBusMessageIter iter;

	if (!dbus_message_iter_init(msg, &iter))
		goto oom;

	return deserialise_object(iter);

oom:
	return log_oom_null();
}
