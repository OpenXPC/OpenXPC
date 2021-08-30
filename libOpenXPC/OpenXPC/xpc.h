#ifndef __XPC_H__
#define __XPC_H__

#include <fcntl.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef HAVE_dispatch
#include <dispatch/dispatch.h>
#endif

__BEGIN_DECLS

#define XPC_TYPE_CONNECTION (&_xpc_type_connection)
#define XPC_TYPE_ENDPOINT (&_xpc_type_endpoint)
#define XPC_TYPE_NULL (&_xpc_type_null)
#define XPC_TYPE_BOOL (&_xpc_type_bool)
#define XPC_BOOL_TRUE (&_xpc_bool_true)
#define XPC_BOOL_FALSE (&_xpc_bool_false)
#define XPC_TYPE_INT64 (&_xpc_type_int64)
#define XPC_TYPE_UINT64 (&_xpc_type_uint64)
#define XPC_TYPE_DOUBLE (&_xpc_type_double)
#define XPC_TYPE_DATE (&_xpc_type_date)
#define XPC_TYPE_DATA (&_xpc_type_data)
#define XPC_TYPE_STRING (&_xpc_type_string)
#define XPC_TYPE_FD (&_xpc_type_fd)
#define XPC_TYPE_SHMEM (&_xpc_type_shmem)
#define XPC_TYPE_ARRAY (&_xpc_type_array)
#define XPC_TYPE_DICTIONARY (&_xpc_type_dictionary)
#define XPC_TYPE_ERROR (&_xpc_type_error)
#ifdef HAVE_uuid
#define XPC_TYPE_UUID (&_xpc_type_uuid)
#endif

#define XPC_ERROR_KEY_DESCRIPTION _xpc_error_key_description
#define XPC_EVENT_KEY_NAME _xpc_event_key_name

#define XPC_ARRAY_APPEND ((size_t)-1)

typedef void *xpc_object_t;
typedef const struct _xpc_type_s *xpc_type_t;
typedef struct _xpc_connection_s *xpc_connection_t;
typedef struct _xpc_endpoint_s *xpc_endpoint_t;
#ifdef __BLOCKS__
typedef void (^xpc_handler_t)(xpc_object_t object);
#endif
typedef void (*xpc_connection_handler_t)(xpc_connection_t connection);

#ifdef __BLOCKS__
typedef bool (^xpc_array_applier_t)(size_t index, xpc_object_t value);
#endif
typedef bool (*xpc_array_applier_fun_t)(size_t index, xpc_object_t value,
	void *udata);

#ifdef __BLOCKS__
typedef bool (^xpc_dictionary_applier_t)(const char *key, xpc_object_t value);
#endif
typedef bool (*xpc_dictionary_applier_fun_t)(const char *key,
	xpc_object_t value, void *udata);

extern const struct _xpc_type_s _xpc_type_connection;
extern const struct _xpc_type_s _xpc_type_endpoint;
extern const struct _xpc_type_s _xpc_type_null;
extern const struct _xpc_type_s _xpc_type_bool;
extern const struct _xpc_bool_s _xpc_bool_true;
extern const struct _xpc_bool_s _xpc_bool_false;
extern const struct _xpc_type_s _xpc_type_int64;
extern const struct _xpc_type_s _xpc_type_uint64;
extern const struct _xpc_type_s _xpc_type_double;
extern const struct _xpc_type_s _xpc_type_date;
extern const struct _xpc_type_s _xpc_type_data;
extern const struct _xpc_type_s _xpc_type_string;
extern const struct _xpc_type_s _xpc_type_fd;
extern const struct _xpc_type_s _xpc_type_shmem;
extern const struct _xpc_type_s _xpc_type_array;
extern const struct _xpc_type_s _xpc_type_dictionary;
extern const struct _xpc_type_s _xpc_type_error;
#ifdef HAVE_uuid
extern const struct _xpc_type_s _xpc_type_uuid;
#endif

extern const char *const _xpc_error_key_description;
extern const char *const _xpc_event_key_name;

#include "connection.h"

xpc_object_t xpc_retain(xpc_object_t object);
void xpc_release(xpc_object_t object);

xpc_type_t xpc_get_type(xpc_object_t object);
xpc_object_t xpc_copy(xpc_object_t object);
bool xpc_equal(xpc_object_t object1, xpc_object_t object2);
size_t xpc_hash(xpc_object_t object);
char *xpc_copy_description(xpc_object_t object);

xpc_object_t xpc_null_create(void);

xpc_object_t xpc_bool_create(bool value);
bool xpc_bool_get_value(xpc_object_t xbool);

xpc_object_t xpc_int64_create(int64_t value);
int64_t xpc_int64_get_value(xpc_object_t xint);

xpc_object_t xpc_uint64_create(uint64_t value);
uint64_t xpc_uint64_get_value(xpc_object_t xuint);

xpc_object_t xpc_double_create(double value);
double xpc_double_get_value(xpc_object_t xdouble);

xpc_object_t xpc_date_create(int64_t interval);
xpc_object_t xpc_date_create_from_current(void);

int64_t xpc_date_get_value(xpc_object_t xdate);

xpc_object_t xpc_data_create(const void *bytes, size_t length);
#if 0
xpc_object_t
xpc_data_create_with_dispatch_data(dispatch_data_t ddata);
#endif
size_t xpc_data_get_length(xpc_object_t xdata);
const void *xpc_data_get_bytes_ptr(xpc_object_t xdata);
size_t xpc_data_get_bytes(xpc_object_t xdata, void *buffer, size_t off,
	size_t length);

xpc_object_t xpc_string_create(const char *string);
xpc_object_t xpc_string_create_with_format(const char *fmt, ...);
xpc_object_t xpc_string_create_with_format_and_arguments(const char *fmt,
	va_list ap);
size_t xpc_string_get_length(xpc_object_t xstring);
const char *xpc_string_get_string_ptr(xpc_object_t xstring);

/*
 * UUIDs
 */
#ifdef HAVE_uuid
xpc_object_t xpc_uuid_create(const uuid_t uuid);
const uint8_t *xpc_uuid_get_bytes(xpc_object_t xuuid);
#endif

xpc_object_t xpc_fd_create(int fd);
int xpc_fd_dup(xpc_object_t xfd);
int xpc_ext_fd_get_fd(xpc_object_t xfd);

xpc_object_t xpc_shmem_create(void *region, size_t length);
size_t xpc_shmem_map(xpc_object_t xshmem, void **region);

/*
 * Arrays
 */

xpc_object_t xpc_array_create(const xpc_object_t *objects, size_t count);
void xpc_array_set_value(xpc_object_t xarray, size_t index, xpc_object_t value);
void xpc_array_append_value(xpc_object_t xarray, xpc_object_t value);
size_t xpc_array_get_count(xpc_object_t xarray);
xpc_object_t xpc_array_get_value(xpc_object_t xarray, size_t index);
#ifdef __BLOCKS__
bool xpc_array_apply(xpc_object_t xarray, xpc_array_applier_t applier);
#endif
bool xpc_array_apply_fun(xpc_object_t xarray, xpc_array_applier_fun_t applier,
	void *udata);
void xpc_array_set_bool(xpc_object_t xarray, size_t index, bool value);
void xpc_array_set_int64(xpc_object_t xarray, size_t index, int64_t value);
void xpc_array_set_uint64(xpc_object_t xarray, size_t index, uint64_t value);
void xpc_array_set_double(xpc_object_t xarray, size_t index, double value);
void xpc_array_set_date(xpc_object_t xarray, size_t index, int64_t value);
void xpc_array_set_data(xpc_object_t xarray, size_t index, const void *bytes,
	size_t length);
void xpc_array_set_string(xpc_object_t xarray, size_t index,
	const char *string);
#ifdef HAVE_uuid
void xpc_array_set_uuid(xpc_object_t xarray, size_t index, const uuid_t uuid);
#endif
void xpc_array_set_fd(xpc_object_t xarray, size_t index, int fd);
bool xpc_array_get_bool(xpc_object_t xarray, size_t index);
int64_t xpc_array_get_int64(xpc_object_t xarray, size_t index);
uint64_t xpc_array_get_uint64(xpc_object_t xarray, size_t index);
double xpc_array_get_double(xpc_object_t xarray, size_t index);
int64_t xpc_array_get_date(xpc_object_t xarray, size_t index);
const void *xpc_array_get_data(xpc_object_t xarray, size_t index,
	size_t *length);
const char *xpc_array_get_string(xpc_object_t xarray, size_t index);
#ifdef HAVE_uuid
const uint8_t *xpc_array_get_uuid(xpc_object_t xarray, size_t index);
#endif
int xpc_array_dup_fd(xpc_object_t xarray, size_t index);

xpc_object_t xpc_dictionary_create(const char *const *keys,
	const xpc_object_t *values, size_t count);
xpc_object_t xpc_dictionary_create_reply(xpc_object_t original);

void xpc_dictionary_set_value(xpc_object_t xdict, const char *key,
	xpc_object_t value);
xpc_object_t xpc_dictionary_get_value(xpc_object_t xdict, const char *key);
size_t xpc_dictionary_get_count(xpc_object_t xdict);
#ifdef __BLOCKS__
bool xpc_dictionary_apply(xpc_object_t xdict, xpc_dictionary_applier_t applier);
#endif
bool xpc_dictionary_apply_fun(xpc_object_t xdict,
	xpc_dictionary_applier_fun_t applier, void *udata);
void xpc_dictionary_set_bool(xpc_object_t xdict, const char *key, bool value);
void xpc_dictionary_set_int64(xpc_object_t xdict, const char *key,
	int64_t value);
void xpc_dictionary_set_uint64(xpc_object_t xdict, const char *key,
	uint64_t value);
void xpc_dictionary_set_double(xpc_object_t xdict, const char *key,
	double value);
void xpc_dictionary_set_date(xpc_object_t xdict, const char *key,
	int64_t value);
void xpc_dictionary_set_data(xpc_object_t xdict, const char *key,
	const void *bytes, size_t length);
void xpc_dictionary_set_string(xpc_object_t xdict, const char *key,
	const char *string);
#ifdef HAVE_uuid
void xpc_dictionary_set_uuid(xpc_object_t xdict, const char *key,
	const uuid_t uuid);
#endif
void xpc_dictionary_set_fd(xpc_object_t xdict, const char *key, int fd);
bool xpc_dictionary_get_bool(xpc_object_t xdict, const char *key);
int64_t xpc_dictionary_get_int64(xpc_object_t xdict, const char *key);
uint64_t xpc_dictionary_get_uint64(xpc_object_t xdict, const char *key);
double xpc_dictionary_get_double(xpc_object_t xdict, const char *key);
int64_t xpc_dictionary_get_date(xpc_object_t xdict, const char *key);
const void *xpc_dictionary_get_data(xpc_object_t xdict, const char *key,
	size_t *length);
const char *xpc_dictionary_get_string(xpc_object_t xdict, const char *key);
#ifdef HAVE_uuid
const uint8_t *xpc_dictionary_get_uuid(xpc_object_t xdict, const char *key);
#endif
int xpc_dictionary_dup_fd(xpc_object_t xdict, const char *key);

__END_DECLS

#endif
