/*-
 * Copyright (c) 2000-2008 Poul-Henning Kamp
 * Copyright (c) 2000-2008 Dag-Erling Coïdan Smørgrav
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer
 *    in this position and unchanged.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *      $FreeBSD$
 */

#ifndef SBUF_H_
#define SBUF_H_

#include <stdarg.h>
#include <unistd.h>

/*
 * We rename our bundled libsbuf's functions to avoid a conflict with macOS'
 * sbuf.h, which is included unconditionally as part of the Darwin module.
 */
#define sbuf sbuf2
#define sbuf_new sbuf2_new
#define sbuf_setpos sbuf2_setpos
#define sbuf_bcat sbuf2_bcat
#define sbuf_bcpy sbuf2_bcpy
#define sbuf_cat sbuf2_cat
#define sbuf_cpy sbuf2_cpy
#define sbuf_printf sbuf2_printf
#define sbuf_vprintf sbuf2_vprintf
#define sbuf_putc sbuf2_putc
#define sbuf_clear sbuf2_clear
#define sbuf_trim sbuf2_trim
#define sbuf_finish sbuf2_finish
#define sbuf_len sbuf2_len
#define sbuf_done sbuf2_done
#define sbuf_delete sbuf2_delete
#define sbuf_data sbuf2_data

#ifndef __printflike
#define __printflike(x, y) __attribute__((format(printf, x, y)))
#endif

// struct sbuf;
typedef int(sbuf_drain_func)(void *, const char *, int);

/*
 * Structure definition
 */
struct sbuf {
	char *s_buf; /* storage buffer */
	sbuf_drain_func *s_drain_func; /* drain function */
	void *s_drain_arg; /* user-supplied drain argument */
	int s_error; /* current error code */
	ssize_t s_size; /* size of storage buffer */
	ssize_t s_len; /* current length of string */
#define SBUF_FIXEDLEN 0x00000000 /* fixed length buffer (default) */
#define SBUF_AUTOEXTEND 0x00000001 /* automatically extend buffer */
#define SBUF_USRFLAGMSK 0x0000ffff /* mask of flags the user may specify */
#define SBUF_DYNAMIC 0x00010000 /* s_buf must be freed */
#define SBUF_FINISHED 0x00020000 /* set by sbuf_finish() */
#define SBUF_DYNSTRUCT 0x00080000 /* sbuf must be freed */
#define SBUF_INSECTION 0x00100000 /* set by sbuf_start_section() */
	int s_flags; /* flags */
	ssize_t s_sect_len; /* current length of section */
};

__BEGIN_DECLS
/*
 * API functions
 */
struct sbuf *sbuf_new(struct sbuf *, char *, int, int);
#define sbuf_new_auto() sbuf_new(NULL, NULL, 0, SBUF_AUTOEXTEND)
void sbuf_clear(struct sbuf *);
int sbuf_setpos(struct sbuf *, ssize_t);
int sbuf_bcat(struct sbuf *, const void *, size_t);
int sbuf_bcpy(struct sbuf *, const void *, size_t);
int sbuf_cat(struct sbuf *, const char *);
int sbuf_cpy(struct sbuf *, const char *);
int sbuf_printf(struct sbuf *, const char *, ...) __printflike(2, 3);
int sbuf_vprintf(struct sbuf *, const char *, va_list) __printflike(2, 0);
int sbuf_putc(struct sbuf *, int);
void sbuf_set_drain(struct sbuf *, sbuf_drain_func *, void *);
int sbuf_trim(struct sbuf *);
int sbuf_error(const struct sbuf *);
int sbuf_finish(struct sbuf *);
char *sbuf_data(struct sbuf *);
ssize_t sbuf_len(struct sbuf *);
int sbuf_done(const struct sbuf *);
void sbuf_delete(struct sbuf *);
void sbuf_start_section(struct sbuf *, ssize_t *);
ssize_t sbuf_end_section(struct sbuf *, ssize_t, size_t, int);
__END_DECLS

#endif /* SBUF_H_ */
