/* readexe - Prints EXE info a la objdump/dumpbin/efd 
 * 
 * Copyright © 2019 Kirn Gill II <segin2005@gmail.com>
 * 
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES 
 * OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE 
 * FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY 
 * DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER 
 * IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING 
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdarg.h>
#include <stdint.h>
#include <stdnoreturn.h>
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif


#ifndef ERR_H
#define ERR_H

#undef NEED_PROGNAME

#ifndef HAVE_SETPROGNAME
#define NEED_PROGNAME
#endif

#ifdef NEED_PROGNAME
extern char *__progname;
#endif 

void vwarnc(int code, const char *format, va_list ap);
noreturn void verrc(int status, int code, const char *format, va_list ap);
void warn(const char *format, ...);
void vwarnx(const char *format, va_list ap);
void warnx(const char *format, ...);
void err(int status, const char *format, ...);
noreturn void verrx(int eval, const char *format, va_list ap);
void errx(int eval, const char *format, ...);

#ifdef NEED_PROGNAME
void setprogname(char *progname);
const char *getprogname(void);
#endif /* NEED_PROGNAME */
#endif /* ERR_H */
