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

#ifndef _LIBXPC_XPC_INTERNAL_H
#define _LIBXPC_XPC_INTERNAL_H

#include <sys/uio.h>
#ifdef HAVE_dispatch
#include <dispatch/dispatch.h>
#endif

#include "xpc2/xpc.h"

#include "compat_queue.h"

#ifdef XPC_DEBUG
#define debugf(...)                                                            \
	do {                                                                   \
		fprintf(stderr, "%s: ", __func__);                             \
		fprintf(stderr, __VA_ARGS__);                                  \
		fprintf(stderr, "\n");                                         \
	} while (0);
#else
#define debugf(...)
#endif

#define XPC_SEQID "XPC sequence number"
#define XPC_PROTOCOL_VERSION 1

struct xpc_object;
struct xpc_dict_pair;
struct xpc_resource;
struct xpc_credentials;

TAILQ_HEAD(xpc_dict_head, xpc_dict_pair);
TAILQ_HEAD(xpc_array_head, xpc_object);

typedef void *xpc_port_t;

typedef union {
	struct xpc_dict_head dict;
	struct xpc_array_head array;
	uint64_t ui;
	int64_t i;
	char *str;
	bool b;
	double d;
	uintptr_t ptr;
	int fd;
#ifdef HAVE_uuid
	uuid_t uuid;
#endif
#ifdef MACH
	mach_port_t port;
#endif
} xpc_u;

#define _XPC_FROM_WIRE 0x1
struct xpc_object {
	xpc_type_t xo_xpc_type;
	uint16_t xo_flags;
	volatile uint32_t xo_refcnt;
	size_t xo_size;
	xpc_u xo_u;
#ifdef MACH
	audit_token_t *xo_audit_token;
#endif
	TAILQ_ENTRY(xpc_object) xo_link;
};

struct xpc_dict_pair {
	const char *key;
	struct xpc_object *value;
	TAILQ_ENTRY(xpc_dict_pair) xo_link;
};

struct xpc_resource {
	int xr_type;
#define XPC_RESOURCE_FD 0x01
#define XPC_RESOURCE_SHMEM 0x02
	union {
		int xr_fd;
	};
};

struct _xpc_type_s {
	const char *description;
};

#define xo_str xo_u.str
#define xo_bool xo_u.b
#define xo_uint xo_u.ui
#define xo_int xo_u.i
#define xo_ptr xo_u.ptr
#define xo_d xo_u.d
#define xo_fd xo_u.fd
#define xo_uuid xo_u.uuid
#define xo_port xo_u.port
#define xo_array xo_u.array
#define xo_dict xo_u.dict

__private_extern__ struct xpc_transport *xpc_get_transport(void);
__private_extern__ void xpc_set_transport(struct xpc_transport *);
__private_extern__ struct xpc_object *_xpc_prim_create(xpc_type_t type,
	xpc_u value, size_t size);
__private_extern__ struct xpc_object *_xpc_prim_create_flags(xpc_type_t type,
	xpc_u value, size_t size, uint16_t flags);
__private_extern__ const char *_xpc_get_type_name(xpc_object_t obj);
__private_extern__ void xpc_object_destroy(struct xpc_object *xo);

#endif /* _LIBXPC_XPC_INTERNAL_H */
