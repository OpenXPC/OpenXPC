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

#include <sys/stat.h>

#include <dirent.h>
#include <glob.h>
#include <limits.h>

#include <Foundation/Foundation.h>

#include "xpc2/log.h"

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

static int validate(NSMutableDictionary<NSString *, id> *dict,
	schema_entry_t schema[], void *udata1, void *udata2)
{
	__block int r = 0;

	[dict enumerateKeysAndObjectsUsingBlock:^(NSString *key, id val,
		BOOL *stop) {
		schema_entry_t *entry = NULL;

		for (int i = 0;; i++) {
			if (schema[i].key == NULL)
				break;
			if (!strcmp([key UTF8String], schema[i].key))
				entry = &schema[i];
		}

		if (!entry)

			log_error("Unmatched key %s\n", [key UTF8String]);
	}];

	return r;
}

static int cb_progargs(const char *key, id val, void *udata1, void *udata2)
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

/* clang-format off */
schema_entry_t entries[] = {
	{ "Label",		kString,	0, 	kRequired },
	{ "Disabled",		kBool,		},
	{ "Program",		kString,	},
	{ "ProgramArguments",	kArray,		cb_progargs	},
	{ NULL },
};
/* clang-format on */

static void load(const char *path);

static void parsefile(NSMutableDictionary *dict)
{
	validate(dict, entries, NULL, NULL);
}

static void loadfile(const char *path)
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
}

static void loaddir(const char *path)
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

static void load(const char *path)
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

int main(int argc, char *argv[])
{
	for (int i = 1; i < argc; i++) {
		load(argv[i]);
	}
	return 0;
}
