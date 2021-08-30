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

#ifndef LOG_H_
#define LOG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <errno.h>
#include <stdarg.h>

#define PRINTFLIKE(fmt, args) __attribute__((format(printf, fmt, args)))

enum log_level { kDebug, kInfo, kNotice, kWarn, kError, kMax };

int log_oom(void);

int ldr_log(int level, const char *file, int line, const char *func,
	const char *format, ...) PRINTFLIKE(5, 6);
/** Logs, then returns -error. */
int ldr_log_errno(int level, int error, const char *file, int line,
	const char *func, const char *fmt, ...) PRINTFLIKE(6, 7);
int ldr_logv(int level, const char *file, int line, const char *func,
	const char *format, va_list ap) PRINTFLIKE(5, 0);

#define log_oom_null() (log_oom(), NULL);
#define log_debug(...)                                                         \
	ldr_log(kDebug, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define log_info(...) ldr_log(kInfo, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define log_notice(...)                                                        \
	ldr_log(kNotice, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define log_warning(...)                                                       \
	ldr_log(kWarn, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define log_error(...)                                                         \
	ldr_log(kError, __FILE__, __LINE__, __func__, __VA_ARGS__)

#define log_debug_errno(error, ...)                                            \
	ldr_log_errno(kDebug, error, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define log_info_errno(error, ...)                                             \
	ldr_log_errno(kInfo, error, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define log_notice_errno(error, ...)                                           \
	ldr_log_errno(kNotice, error, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define log_warning_errno(error, ...)                                          \
	ldr_log_errno(kWarn, error, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define log_error_errno(error, ...)                                            \
	ldr_log_errno(kError, error, __FILE__, __LINE__, __func__, __VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif /* LOG_H_ */
