/*
 * Copyright 2014-2015 iXsystems, Inc.
 * All rights reserved
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted providing that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

#define _GNU_SOURCE

#include <sys/types.h>

#include <stdio.h>
#include <time.h>

#include "OpenXPC/xpc.h"
#include "xpc_internal.h"

typedef const struct _xpc_type_s xt;
xt _xpc_type_invalid = { "<invalid>" };
xt _xpc_type_array = { "array" };
xt _xpc_type_bool = { "bool" };
xt _xpc_type_connection = { "connection" };
xt _xpc_type_data = { "data" };
xt _xpc_type_date = { "date" };
xt _xpc_type_dictionary = { "dictionary" };
xt _xpc_type_endpoint = { "endpoint" };
xt _xpc_type_null = { "null" };
xt _xpc_type_error = { "error" };
xt _xpc_type_fd = { "file descriptor" };
xt _xpc_type_int64 = { "int64" };
xt _xpc_type_uint64 = { "uint64" };
xt _xpc_type_shmem = { "shared memory" };
xt _xpc_type_string = { "string" };
#ifdef HAVE_uuid
xt _xpc_type_uuid = { "UUID" };
#endif
xt _xpc_type_double = { "double" };

struct _xpc_bool_s {
	struct xpc_object object;
};

typedef struct _xpc_bool_s xb;

xb _xpc_bool_true = { .object.xo_xpc_type = XPC_TYPE_BOOL,
	.object.xo_u = { .b = true } };
xb _xpc_bool_false = { .object.xo_xpc_type = XPC_TYPE_BOOL,
	.object.xo_u = { .b = false } };
;

struct _xpc_dictionary_s {
};

typedef const struct _xpc_dictionary_s xs;

static size_t xpc_data_hash(const uint8_t *data, size_t length);

__private_extern__ struct xpc_object *
_xpc_prim_create(xpc_type_t type, xpc_u value, size_t size)
{
	return (_xpc_prim_create_flags(type, value, size, 0));
}

__private_extern__ struct xpc_object *
_xpc_prim_create_flags(xpc_type_t type, xpc_u value, size_t size,
	uint16_t flags)
{
	struct xpc_object *xo;

	if ((xo = malloc(sizeof(*xo))) == NULL)
		return (NULL);

	xo->xo_size = size;
	xo->xo_xpc_type = type;
	xo->xo_flags = flags;
	xo->xo_u = value;
	xo->xo_refcnt = 1;
#if MACH
	xo->xo_audit_token = NULL;
#endif

	if (type == XPC_TYPE_DICTIONARY)
		TAILQ_INIT(&xo->xo_dict);

	if (type == XPC_TYPE_ARRAY)
		TAILQ_INIT(&xo->xo_array);

	return (xo);
}

xpc_object_t
xpc_null_create(void)
{
	xpc_u val;
	val.ui = 0;
	return _xpc_prim_create(XPC_TYPE_NULL, val, 0);
}

xpc_object_t
xpc_bool_create(bool value)
{
	xpc_u val;

	val.b = value;
	return _xpc_prim_create(XPC_TYPE_BOOL, val, 1);
}

bool
xpc_bool_get_value(xpc_object_t xbool)
{
	struct xpc_object *xo;

	xo = xbool;
	if (xo == NULL)
		return (0);

	if (xo->xo_xpc_type == XPC_TYPE_BOOL)
		return (xo->xo_bool);

	return (false);
}

xpc_object_t
xpc_int64_create(int64_t value)
{
	xpc_u val;

	val.i = value;
	return _xpc_prim_create(XPC_TYPE_INT64, val, 1);
}

int64_t
xpc_int64_get_value(xpc_object_t xint)
{
	struct xpc_object *xo;

	xo = xint;
	if (xo == NULL)
		return (0);

	if (xo->xo_xpc_type == XPC_TYPE_INT64)
		return (xo->xo_int);

	return (0);
}

xpc_object_t
xpc_uint64_create(uint64_t value)
{
	xpc_u val;

	val.ui = value;
	return _xpc_prim_create(XPC_TYPE_UINT64, val, 1);
}

uint64_t
xpc_uint64_get_value(xpc_object_t xuint)
{
	struct xpc_object *xo;

	xo = xuint;
	if (xo == NULL)
		return (0);

	if (xo->xo_xpc_type == XPC_TYPE_UINT64)
		return (xo->xo_uint);

	return (0);
}

xpc_object_t
xpc_double_create(double value)
{
	xpc_u val;

	val.d = value;
	return _xpc_prim_create(XPC_TYPE_DOUBLE, val, 1);
}

double
xpc_double_get_value(xpc_object_t xdouble)
{
	struct xpc_object *xo = xdouble;

	if (xo->xo_xpc_type == XPC_TYPE_DOUBLE)
		return (xo->xo_d);

	return (0);
}

xpc_object_t
xpc_date_create(int64_t interval)
{
	xpc_u val;

	val.i = interval;
	return _xpc_prim_create(XPC_TYPE_DATE, val, 1);
}

xpc_object_t
xpc_date_create_from_current(void)
{
	xpc_u val;
	struct timespec tp;

	clock_gettime(CLOCK_REALTIME, &tp);

	val.ui = *(uint64_t *)&tp;
	return _xpc_prim_create(XPC_TYPE_DATE, val, 1);
}

int64_t
xpc_date_get_value(xpc_object_t xdate)
{
	struct xpc_object *xo = xdate;

	if (xo == NULL)
		return (0);

	if (xo->xo_xpc_type == XPC_TYPE_DATE)
		return (xo->xo_int);

	return (0);
}

xpc_object_t
xpc_data_create(const void *bytes, size_t length)
{
	xpc_u val;

	val.ptr = (uintptr_t)malloc(length);
	memcpy((void *)val.ptr, bytes, length);
	return _xpc_prim_create(XPC_TYPE_DATA, val, length);
}

xpc_object_t
xpc_data_create_nocopy(void *bytes, size_t length)
{
	xpc_u val;

	val.ptr = (uintptr_t)bytes;
	return _xpc_prim_create(XPC_TYPE_DATA, val, length);
}

#ifdef MACH
xpc_object_t
xpc_data_create_with_dispatch_data(dispatch_data_t ddata)
{
}
#endif

size_t
xpc_data_get_length(xpc_object_t xdata)
{
	struct xpc_object *xo = xdata;

	if (xo == NULL)
		return (0);

	if (xo->xo_xpc_type == XPC_TYPE_DATA)
		return (xo->xo_size);

	return (0);
}

const void *
xpc_data_get_bytes_ptr(xpc_object_t xdata)
{
	struct xpc_object *xo = xdata;

	if (xo == NULL)
		return (NULL);

	if (xo->xo_xpc_type == XPC_TYPE_DATA)
		return ((const void *)xo->xo_ptr);

	return (0);
}

size_t
xpc_data_get_bytes(xpc_object_t xdata, void *buffer, size_t off, size_t length)
{
	/* XXX */
	return (0);
}

xpc_object_t
xpc_string_create(const char *string)
{
	xpc_u val;

	val.str = strdup(string);
	return _xpc_prim_create(XPC_TYPE_STRING, val, strlen(string));
}

xpc_object_t
xpc_string_create_with_format(const char *fmt, ...)
{
	va_list ap;
	xpc_u val;

	va_start(ap, fmt);
	vasprintf(&val.str, fmt, ap);
	va_end(ap);
	return _xpc_prim_create(XPC_TYPE_STRING, val, strlen(val.str));
}

xpc_object_t
xpc_string_create_with_format_and_arguments(const char *fmt, va_list ap)
{
	xpc_u val;

	vasprintf(&val.str, fmt, ap);
	return _xpc_prim_create(XPC_TYPE_STRING, val, strlen(val.str));
}

size_t
xpc_string_get_length(xpc_object_t xstring)
{
	struct xpc_object *xo = xstring;

	if (xo == NULL)
		return (0);

	if (xo->xo_xpc_type == XPC_TYPE_STRING)
		return (xo->xo_size);

	return (0);
}

const char *
xpc_string_get_string_ptr(xpc_object_t xstring)
{
	struct xpc_object *xo = xstring;

	if (xo == NULL)
		return (NULL);

	if (xo->xo_xpc_type == XPC_TYPE_STRING)
		return (xo->xo_str);

	return (NULL);
}

xpc_object_t
xpc_fd_create(int fd)
{
	xpc_u val;

	val.fd = dup(fd);
	return _xpc_prim_create(XPC_TYPE_FD, val, 1);
}

int
xpc_ext_fd_get_fd(xpc_object_t xfd)
{
	struct xpc_object *xo;

	xo = xfd;
	if (xo == NULL)
		return (-1);

	if (xo->xo_xpc_type == XPC_TYPE_FD)
		return (xo->xo_fd);

	return (-1);
}

#ifdef HAVE_uuid
xpc_object_t
xpc_uuid_create(const uuid_t uuid)
{
	xpc_u val;

	memcpy(val.uuid, uuid, sizeof(uuid_t));
	return _xpc_prim_create(XPC_TYPE_UUID, val, 1);
}

const uint8_t *
xpc_uuid_get_bytes(xpc_object_t xuuid)
{
	struct xpc_object *xo;

	xo = xuuid;
	if (xo == NULL)
		return (NULL);

	if (xo->xo_xpc_type == XPC_TYPE_UUID)
		return ((uint8_t *)&xo->xo_uuid);

	return (NULL);
}
#endif

xpc_type_t
xpc_get_type(xpc_object_t obj)
{
	struct xpc_object *xo;

	xo = obj;
	return (xo->xo_xpc_type);
}

bool
xpc_equal(xpc_object_t x1, xpc_object_t x2)
{
	struct xpc_object *xo1, *xo2;

	xo1 = x1;
	xo2 = x2;

	/* FIXME */
	return (false);
}

static bool
dict_copy_cb(const char *k, xpc_object_t v, void *udata)
{
	struct xpc_object *xotmp = udata;
	xpc_dictionary_set_value(xotmp, k, xpc_copy(v));
	return (bool)true;
}

static bool
array_copy_cb(size_t idx, xpc_object_t v, void *udata)
{
	struct xpc_object *xotmp = udata;
	xpc_array_set_value(xotmp, idx, xpc_copy(v));
	return ((bool)true);
}

xpc_object_t
xpc_copy(xpc_object_t obj)
{
	struct xpc_object *xo, *xotmp;
	const void *newdata;

	xo = obj;
	if (xo->xo_xpc_type == XPC_TYPE_BOOL ||
		xo->xo_xpc_type == XPC_TYPE_INT64 ||
		xo->xo_xpc_type == XPC_TYPE_UINT64 ||
		xo->xo_xpc_type == XPC_TYPE_DATE ||
		xo->xo_xpc_type == XPC_TYPE_ENDPOINT)
		return _xpc_prim_create(xo->xo_xpc_type, xo->xo_u, 1);
	else if (xo->xo_xpc_type == XPC_TYPE_STRING)
		return xpc_string_create(strdup(xpc_string_get_string_ptr(xo)));
	else if (xo->xo_xpc_type == XPC_TYPE_DATA) {
		newdata = xpc_data_get_bytes_ptr(obj);
		return (xpc_data_create(newdata, xpc_data_get_length(obj)));
	} else if (xo->xo_xpc_type == XPC_TYPE_DICTIONARY) {
		xotmp = xpc_dictionary_create(NULL, NULL, 0);
		xpc_dictionary_apply_fun(obj, dict_copy_cb, xotmp);
		return (xotmp);
	} else if (xo->xo_xpc_type == XPC_TYPE_ARRAY) {
		xotmp = xpc_array_create(NULL, 0);
		xpc_array_apply_fun(obj, array_copy_cb, xotmp);
		return (xotmp);
	}

	return (0);
}

static size_t
xpc_data_hash(const uint8_t *data, size_t length)
{
	size_t hash = 5381;

	while (length--)
		hash = ((hash << 5) + hash) + data[length];

	return (hash);
}

static bool
dict_hash_cb(const char *k, xpc_object_t v, void *udata)
{
	size_t *hash = udata;
	*hash ^= xpc_data_hash((const uint8_t *)k, strlen(k));
	*hash ^= xpc_hash(v);
	return (bool)true;
}

static bool
array_hash_cb(size_t idx, xpc_object_t v, void *udata)
{
	size_t *hash = udata;
	*hash ^= xpc_hash(v);
	return ((bool)true);
}

size_t
xpc_hash(xpc_object_t obj)
{
	struct xpc_object *xo;
	size_t hash = 0;

	xo = obj;
	if (xo->xo_xpc_type == XPC_TYPE_BOOL ||
		xo->xo_xpc_type == XPC_TYPE_INT64 ||
		xo->xo_xpc_type == XPC_TYPE_UINT64 ||
		xo->xo_xpc_type == XPC_TYPE_DATE ||
		xo->xo_xpc_type == XPC_TYPE_ENDPOINT)
		return ((size_t)xo->xo_u.ui);

	else if (xo->xo_xpc_type == XPC_TYPE_STRING)
		return (xpc_data_hash(
			(const uint8_t *)xpc_string_get_string_ptr(obj),
			xpc_string_get_length(obj)));

	else if (xo->xo_xpc_type == XPC_TYPE_DATA)
		return (xpc_data_hash(xpc_data_get_bytes_ptr(obj),
			xpc_data_get_length(obj)));

	else if (xo->xo_xpc_type == XPC_TYPE_DICTIONARY) {
		xpc_dictionary_apply_fun(obj, dict_hash_cb, &hash);
		return (hash);
	} else if (xo->xo_xpc_type == XPC_TYPE_ARRAY) {
		xpc_array_apply_fun(obj, array_hash_cb, &hash);
		return (hash);
	}

	return (0);
}

__private_extern__ const char *
_xpc_get_type_name(xpc_object_t obj)
{
	struct xpc_object *xo;

	xo = obj;
	return xo->xo_xpc_type->description;
}
