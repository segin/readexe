#include <stdarg.h>
#include <stdint.h>

#ifndef ERR_H
#define ERR_H

#define NEED_ERR

extern char *__progname;

void vwarnc(int code, const char *format, va_list ap);
void verrc(int status, int code, const char *format, va_list ap);
void warn(const char *format, ...);
void vwarnx(const char *format, va_list ap);
void warnx(const char *format, ...);
void err(int status, const char *format, ...);
void verrx(int eval, const char *format, va_list ap);
void errx(int eval, const char *format, ...);
void setprogname(const char *progname);
const char *getprogname();

#endif 