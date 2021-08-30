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

#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "OpenXPC/log.h"

int
log_oom()
{
	fprintf(stderr, "Out of memory.\n");
	return -ENOMEM;
}

/* Expand %m. fmtout must be a char [LINE_MAX]. */
static int
expand_percentm(int errno_num, const char *fmtin, char *fmtout)
{
	char *fmtout_end = &fmtout[LINE_MAX];

	while (*fmtin != '\0' && fmtout < fmtout_end)
		if (*fmtin == '%' && fmtin[1] == 'm') {
			fmtin += 2;
			fmtout += snprintf(fmtout, fmtout_end - fmtout, "%s",
				strerror(errno_num));
		} else
			*fmtout++ = *fmtin++;
	*fmtout = '\0';

	return 0;
}

static int
log_output(int level, const char *file, int line, const char *func,
	const char *object_name, const char *object, char *buffer)
{
	if ((0))
		printf("(%s:%u) ", file, line);

	if (level == kError)
		printf("\x1B[1;31m");

	printf("%s", buffer);

	if (level == kError)
		printf("\x1B[0m");

	return printf("\n");
}

int
ldr_logv(int level, const char *file, int line, const char *func,
	const char *fmt, va_list ap)
{
	char fmtcpy[LINE_MAX];
	char buffer[LINE_MAX];
	int siz;

	expand_percentm(errno, fmt, fmtcpy);
	siz = vsnprintf(buffer, (sizeof buffer) - 1, fmtcpy, ap);

	if (buffer[siz - 1] == '\n')
		buffer[siz - 1] = '\0';

	return log_output(level, file, line, func, NULL, NULL, buffer);
}

int
ldr_log(int level, const char *file, int line, const char *func,
	const char *fmt, ...)
{
	int r;
	va_list ap;

	va_start(ap, fmt);
	r = ldr_logv(level, file, line, func, fmt, ap);
	va_end(ap);

	return r;
}

int
ldr_log_errno(int level, int error, const char *file, int line,
	const char *func, const char *fmt, ...)
{
	int r;
	va_list ap;

	errno = error;

	va_start(ap, fmt);
	r = ldr_logv(level, file, line, func, fmt, ap);
	va_end(ap);

	errno = error;
	return -error;
}
