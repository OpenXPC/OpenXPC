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

#include <sys/types.h>
#include <sys/errno.h>

#include <assert.h>
#include <inttypes.h>
#include <pthread.h>
#include <syslog.h>

#include "OpenXPC/xpc.h"
#include "compat_sbuf.h"
#include "xpc_internal.h"

#define atomic_fetchadd_int(p, v) __sync_fetch_and_add(p, v)
#define atomic_add_int(p, v) (void)__sync_fetch_and_add(p, v)

static void xpc_copy_description_level(xpc_object_t obj, struct sbuf *sbuf,
	int level);

static void
xpc_dictionary_destroy(struct xpc_object *dict)
{
	struct xpc_dict_head *head;
	struct xpc_dict_pair *p, *ptmp;

	head = &dict->xo_dict;

	TAILQ_FOREACH_SAFE(p, head, xo_link, ptmp)
	{
		TAILQ_REMOVE(head, p, xo_link);
		xpc_object_destroy(p->value);
		free(p);
	}
}

static void
xpc_array_destroy(struct xpc_object *dict)
{
	struct xpc_object *p, *ptmp;
	struct xpc_array_head *head;

	head = &dict->xo_array;

	TAILQ_FOREACH_SAFE(p, head, xo_link, ptmp)
	{
		TAILQ_REMOVE(head, p, xo_link);
		xpc_object_destroy(p);
	}
}

void
xpc_object_destroy(struct xpc_object *xo)
{
	if (xo->xo_xpc_type == XPC_TYPE_DICTIONARY)
		xpc_dictionary_destroy(xo);

	if (xo->xo_xpc_type == XPC_TYPE_ARRAY)
		xpc_array_destroy(xo);

	free(xo);
}

xpc_object_t
xpc_retain(xpc_object_t obj)
{
	struct xpc_object *xo;

	xo = obj;
	atomic_add_int(&xo->xo_refcnt, 1);
	return (obj);
}

void
xpc_release(xpc_object_t obj)
{
	struct xpc_object *xo;

	xo = obj;
	if (atomic_fetchadd_int(&xo->xo_refcnt, -1) > 1)
		return;

	xpc_object_destroy(xo);
}

static const char *xpc_errors[] = { "No Error Found", "No Memory",
	"Invalid Argument", "No Such Process" };

#if 0
const char *
xpc_strerror(int error)
{

	if (error > EXMAX || error < 0)
		return "BAD ERROR";
	return (xpc_errors[error]);
}
#endif

char *
xpc_copy_description(xpc_object_t obj)
{
	char *result;
	struct sbuf *sbuf;

	sbuf = sbuf_new_auto();
	xpc_copy_description_level(obj, sbuf, 0);
	sbuf_finish(sbuf);
	result = strdup(sbuf_data(sbuf));
	sbuf_delete(sbuf);

	return (result);
}

static void
xpc_copy_description_level(xpc_object_t obj, struct sbuf *sbuf, int level)
{
	struct xpc_object *xo = obj;
#ifdef HAVE_uuid
	struct uuid *id;
	char *uuid_str;
	uint32_t uuid_status;
#endif

	if (obj == NULL) {
		sbuf_printf(sbuf, "<null value>\n");
		return;
	}

	sbuf_printf(sbuf, "(%s) ", _xpc_get_type_name(obj));

	if (xo->xo_xpc_type == XPC_TYPE_DICTIONARY) {
		sbuf_printf(sbuf, "\n");
		xpc_dictionary_apply(xo, ^(const char *k, xpc_object_t v) {
			sbuf_printf(sbuf, "%*s\"%s\": ", level * 4, " ", k);
			xpc_copy_description_level(v, sbuf, level + 1);
			return ((bool)true);
		});
	} else if (xo->xo_xpc_type == XPC_TYPE_ARRAY) {
		sbuf_printf(sbuf, "\n");
		xpc_array_apply(xo, ^(size_t idx, xpc_object_t v) {
			sbuf_printf(sbuf, "%*s%ld: ", level * 4, " ", idx);
			xpc_copy_description_level(v, sbuf, level + 1);
			return ((bool)true);
		});
	} else if (xo->xo_xpc_type == XPC_TYPE_BOOL)
		sbuf_printf(sbuf, "%s\n",
			xpc_bool_get_value(obj) ? "true" : "false");
	else if (xo->xo_xpc_type == XPC_TYPE_STRING)
		sbuf_printf(sbuf, "\"%s\"\n", xpc_string_get_string_ptr(obj));
	else if (xo->xo_xpc_type == XPC_TYPE_INT64)
		sbuf_printf(sbuf, "0x%" PRIi64 "\n", xpc_int64_get_value(obj));
	else if (xo->xo_xpc_type == XPC_TYPE_UINT64)
		sbuf_printf(sbuf, "0x%" PRIx64 "\n", xpc_uint64_get_value(obj));
	else if (xo->xo_xpc_type == XPC_TYPE_DATE) {
		sbuf_printf(sbuf, "%" PRIu64 "\n", xpc_date_get_value(obj));
#ifdef HAVE_uuid
	} else if (xo->xo_xpc_type == XPC_TYPE_UUID) {
		uuid_t id;
		uuid_string_t uuid_str;
		memcpy(id, xpc_uuid_get_bytes(obj), sizeof(uuid_t));
		uuid_unparse_upper(id, uuid_str);
		sbuf_printf(sbuf, "%s\n", uuid_str);
		free(uuid_str);
#endif
	} else if (xo->xo_xpc_type == XPC_TYPE_ENDPOINT)
		sbuf_printf(sbuf, "<%" PRIx64 ">\n", xo->xo_int);
	else if (xo->xo_xpc_type == XPC_TYPE_NULL)
		sbuf_printf(sbuf, "<null>\n");
	else if (xo->xo_xpc_type == XPC_TYPE_FD)
		sbuf_printf(sbuf, "<%d>\n", xo->xo_fd);
	else
		sbuf_printf(sbuf, "<ERROR-TYPE!>\n");
}

#ifdef MACH
struct _launch_data {
	uint64_t type;
	union {
		struct {
			union {
				launch_data_t *_array;
				char *string;
				void *opaque;
				int64_t __junk;
			};
			union {
				uint64_t _array_cnt;
				uint64_t string_len;
				uint64_t opaque_size;
			};
		};
		int64_t fd;
		uint64_t mp;
		uint64_t err;
		int64_t number;
		uint64_t boolean; /* We'd use 'bool' but this struct needs to be used under Rosetta, and sizeof(bool) is different between PowerPC and Intel */
		double float_num;
	};
};

static uint8_t ld_to_xpc_type[] = { XPC_TYPE_INVALID, XPC_TYPE_DICTIONARY,
	XPC_TYPE_ARRAY, XPC_TYPE_FD, XPC_TYPE_UINT64, XPC_TYPE_DOUBLE,
	XPC_TYPE_BOOL, XPC_TYPE_STRING, XPC_TYPE_DATA, XPC_TYPE_ERROR,
	XPC_TYPE_ENDPOINT };

xpc_object_t
ld2xpc(launch_data_t ld)
{
	struct xpc_object *xo;
	xpc_u val;

	if (ld->type > LAUNCH_DATA_MACHPORT)
		return (NULL);
	if (ld->type == LAUNCH_DATA_STRING || ld->type == LAUNCH_DATA_OPAQUE) {
		val.str = malloc(ld->string_len);
		memcpy(__DECONST(void *, val.str), ld->string, ld->string_len);
		xo = _xpc_prim_create(ld_to_xpc_type[ld->type], val,
			ld->string_len);
	} else if (ld->type == LAUNCH_DATA_BOOL) {
		val.b = (bool)ld->boolean;
		xo = _xpc_prim_create(ld_to_xpc_type[ld->type], val, 0);
	} else if (ld->type == LAUNCH_DATA_ARRAY) {
		xo = xpc_array_create(NULL, 0);
		for (uint64_t i = 0; i < ld->_array_cnt; i++)
			xpc_array_append_value(xo, ld2xpc(ld->_array[i]));
	} else {
		val.ui = ld->mp;
		xo = _xpc_prim_create(ld_to_xpc_type[ld->type], val,
			ld->string_len);
	}
	return (xo);
}
#endif

#if 0
xpc_object_t
xpc_copy_entitlement_for_token(const char *key __unused, audit_token_t *token __unused)
{
	xpc_u val;

	val.b = true;
	return (_xpc_prim_create(XPC_TYPE_BOOL, val,0));
}
#endif
