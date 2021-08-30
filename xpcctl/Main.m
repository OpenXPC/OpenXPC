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

#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>

#include <dirent.h>
#include <glob.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>

#include <Foundation/Foundation.h>

#include "OpenXPC/log.h"
#include "OpenXPC/xpc.h"
#include "XPC.h"

typedef struct schema_entry schema_entry_t;

typedef int (*schema_cb_t)(const char *key, id val, void *udata1, void *udata2);

struct schema_entry {
	const char *key;
	enum {
		kAny,
		kDictionary,
		kArray,
		kNumber,
		kString,
		kBool,
	} type;
	schema_cb_t cb;
	enum {
		kOptional,
		kRequired,
	} need;
};

static int
validate(NSMutableDictionary<NSString *, id> *dict, schema_entry_t schema[],
	void *udata1, void *udata2)
{
	__block int r = 0;

	[dict enumerateKeysAndObjectsUsingBlock:^(NSString *key, id val,
		BOOL *stop) {
		schema_entry_t *entry = NULL;
		Class cls;

		for (int i = 0;; i++) {
			if (schema[i].key == NULL)
				break;
			if (!strcmp([key UTF8String], schema[i].key))
				entry = &schema[i];
		}

		if (!entry) {
			log_error("Unmatched key %s\n", [key UTF8String]);
			*stop = YES;
			return;
		}

		switch (entry->type) {
		case kDictionary:
			cls = [NSDictionary class];
			break;

		case kArray:
			cls = [NSArray class];
			break;

		case kNumber:
			cls = [NSNumber class];
			break;

		case kString:
			cls = [NSString class];
			break;

		case kBool:
			cls = [NSNumber class];
			break;

		default:
			cls = [NSObject class];
		}

		if (![val isKindOfClass:cls]) {
			log_error("Value for key %s has wrong class",
				[key UTF8String]);
			r = -EINVAL;
			*stop = YES;
			return;
		}

		if (entry->cb)
			if ((r = entry->cb([key UTF8String], val, udata1,
				     udata2)) < 0) {
				*stop = YES;
				return;
			}
	}];

	return r;
}

/* process ProgramArguments */
static int
cb_progargs(const char *key, id val, void *udata1, void *udata2)
{
	NSArray *arr = val;
	__block int r = 0;

	[arr enumerateObjectsUsingBlock:^(id value, NSUInteger idx,
		BOOL *stop) {
		if (![value isKindOfClass:[NSString class]]) {
			log_error("Bad entry in ProgramArguments array");
			*stop = true;
			r = -1;
		}
	}];

	return r;
}

/*
 * process a single Socket dictionary, creating socket and adding to the dict.
 */
static int
process_socket(id val, NSMutableDictionary<NSString *, id> *plist)
{
	NSString *pathname, *typestr;
	char secsockpath[255] = { '\0' };
	NSMutableDictionary<NSString *, id> *sock = val;
	int type = SOCK_STREAM;
	int fd;

	if (![val isKindOfClass:[NSDictionary class]]) {
		log_error("Bad socket entry");
		return -EINVAL;
	}

	if ((typestr = sock[@"SockType"])) {
		if ([typestr isEqualToString:@"dgram"])
			type = SOCK_DGRAM;
		else if ([typestr isEqualToString:@"seqpacket"])
			type = SOCK_SEQPACKET;
		else if ([typestr isEqualToString:@"stream"])
			type = SOCK_STREAM;
		else
			return log_error_errno(EINVAL, "Bad socket type");
	}

	if ((pathname = sock[@"SecureSocketWithKey"])) {
		NSMutableDictionary *userenv;
		char secsockdir[] = "/tmp/xpcd.XXXXXX", *res;

		res = mkdtemp(secsockdir);
		snprintf(secsockpath, sizeof(secsockpath) - 1, "%s/%s",
			secsockdir, [pathname UTF8String]);

		log_info("Generated secure socket <%s>", secsockpath);

		userenv = [plist objectForKey:@"UserEnvironmentVariables"];
		if (!userenv)
			userenv = plist[@"UserEnvironmentVariables"] =
				[NSMutableDictionary new];

		/* add the resultant secure socket path to job's env */
		userenv[pathname] = [NSString stringWithCString:secsockpath];
	} else
		pathname = sock[@"SockPathName"];

	if (pathname) {
		/* unix domain socket */
		struct sockaddr_un sun;

		sun.sun_family = AF_UNIX;
		strncpy(sun.sun_path, [pathname UTF8String],
			sizeof(sun.sun_path));

		if ((fd = socket(AF_UNIX, type, 0)) < 0)
			return log_error_errno(errno,
				"Failed to open socket: %m");

		if ((unlink(sun.sun_path) < 0) && errno != ENOENT) {
			log_error("Failed to remove old socket file: %m");
			close(fd);
			return -errno;
		}

		sock[@"FileDescriptor"] = [[[NSFileHandle alloc]
			initWithFileDescriptor:fd
				closeOnDealloc:NO] autorelease];
	} else {
		// TODO: non-Unix sockets
	}

	return 0;
}

static int
cb_sockets(const char *key, id val, void *udata1, void *udata2)
{
	NSDictionary<NSString *, id> *dict = val;
	__block int r = 0;

	[dict enumerateKeysAndObjectsUsingBlock:^(NSString *key, id val,
		BOOL *stop) {
		if ([val isKindOfClass:[NSArray class]]) {
			NSArray *arr = (NSArray *)val;

			[arr enumerateObjectsUsingBlock:^(id obj,
				NSUInteger idx, BOOL *stop) {
				if ((r = process_socket(obj, udata1)) <= 0)
					*stop = YES;
			}];
		} else if ([val isKindOfClass:[NSDictionary class]]) {
			if ((r = process_socket(val, udata1)) <= 0)
				*stop = YES;
		}
	}];

	return r;
}

/* clang-format off */
schema_entry_t entries[] = {
	{ "Label",		kString,	0, 	kRequired },
	{ "Disabled",		kBool,		},
	{ "UserName",		kString,	},
	{ "GroupName",		kString,	},
	{ "inetdCompatibility",	kDictionary,	},
	{ "Program",		kString,	},
	{ "ProgramArguments",	kArray,		cb_progargs	},
	{ "ServiceIPC",		kBool,		}, /* noop */
	{ "Sockets",		kDictionary,	cb_sockets	},
	{ NULL },
};
/* clang-format on */

static void load(const char *path);

static void
parsefile(NSMutableDictionary *dict)
{
	validate(dict, entries, dict, NULL);
}

static void
loadfile(const char *path)
{
	NSError *error = nil;
	NSData *data = [NSData
		dataWithContentsOfFile:[NSString stringWithUTF8String:path]];
	id plist;

	if (!data)
		return (void)log_error_errno(errno,
			"File %s: Failed to read contents: %m", path);

	plist = [NSPropertyListSerialization
		propertyListWithData:data
			     options:NSPropertyListMutableContainers
			      format:NULL
			       error:&error];

	if (!plist)
		return (void)log_error("File %s: %s", path,
			[[error localizedDescription] UTF8String]);
	else if (![plist isKindOfClass:[NSMutableDictionary class]])
		return (void)log_error(
			"Object not kind of NSMutableDictionary (is %s)",
			class_getName([plist class]));

	parsefile(plist);

	xpc_object_t res = [plist newXPCObject];
	log_error("%s\n", xpc_copy_description(res));

	xpc_connection_t conn = xpc_connection_create_mach_service(
		"org.freedesktop.DBus", NULL, 0);
	xpc_connection_send_message(conn, res);
}

static void
loaddir(const char *path)
{
	struct dirent *de;
	DIR *d;

	if ((d = opendir(path)) == NULL) {
		log_error("opendir() failed to open the directory: %m");
		return;
	}

	while ((de = readdir(d))) {
		char buf[PATH_MAX];

		if (de->d_name[0] == '.')
			continue;

		snprintf(buf, sizeof(buf), "%s/%s", path, de->d_name);
		load(buf);
	}
	closedir(d);
}

static void
load(const char *path)
{
	struct stat st;
	if (stat(path, &st) == -1)
		log_error("Failed to stat %s: %m", path);
	else if (S_ISREG(st.st_mode)) {
		NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
		loadfile(path);
		[pool release];
	} else if (S_ISDIR(st.st_mode))
		loaddir(path);
}

int
main(int argc, char *argv[])
{
	for (int i = 1; i < argc; i++) {
		load(argv[i]);
	}
	return 0;
}
