/*
 * Copyright © 2006 Robert Millan
 * Copyright © 2011, 2019 Guillem Jover <guillem@hadrons.org>
 * Copyright © 2019 Kirn Gill II <segin2005@gmail.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL
 * THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * Imported from libbsd - https://libbsd.freedesktop.org/
 */

#include "err.h"
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#ifndef _WATCOM
# include <stdnoreturn.h>
#else
# define noreturn
#endif

#ifdef NEED_PROGNAME
char *__progname;
#endif

#ifdef _WINCE
static int errno;

char *strerror(int code) { 
	return "";
}
#else
#include <errno.h>
#endif

void
vwarnc(int code, const char *format, va_list ap)
{
	fprintf(stderr, "%s: ", getprogname());
	if (format) {
		vfprintf(stderr, format, ap);
		fprintf(stderr, ": ");
	}
	fprintf(stderr, "%s\n", strerror(code));
}

noreturn void
verrc(int status, int code, const char *format, va_list ap)
{
	vwarnc(code, format, ap);
	exit(status);
}

void
warn(const char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	vwarnc(errno, format, ap);
	va_end(ap);
}

void
vwarnx(const char *format, va_list ap)
{
	fprintf(stderr, "%s: ", getprogname());
	if (format)
		vfprintf(stderr, format, ap);
	fprintf(stderr, "\n");
}

void
warnx(const char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	vwarnx(format, ap);
	va_end(ap);
}

noreturn void
err(int status, const char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	verrc(status, errno, format, ap);
	va_end(ap);
}

noreturn void
verrx(int eval, const char *format, va_list ap)
{
	fprintf(stderr, "%s: ", getprogname());
	if (format)
		vfprintf(stderr, format, ap);
	fprintf(stderr, "\n");
	exit(eval);
}

noreturn void
errx(int eval, const char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	verrx(eval, format, ap);
	va_end(ap);
}

#ifdef NEED_PROGNAME
void setprogname(char *progname) {
    __progname = progname;
}

const char *getprogname(void) {
    return __progname;
}
#endif

