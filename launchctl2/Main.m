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

static void loadfile(const char *path)
{
	log_info("Load %s\n", path);
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
		loadfile(buf);
	}
	closedir(d);
}

int main(int argc, char *argv[])
{
	for (int i = 1; i < argc; i++) {
		struct stat st;
		const char *path = argv[i];

		if (stat(path, &st) == -1)
			log_error("Failed to stat %s: %m", path);
		else if (S_ISREG(st.st_mode))
			loadfile(path);
		else if (S_ISDIR(st.st_mode)) {
			loaddir(path);
		}
	}
	return 0;
}
