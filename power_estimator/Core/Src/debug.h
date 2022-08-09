#ifndef DEBUG_H__
#define DEBUG_H__

#include <stdarg.h>

void dbgprintf(const char *fmt,...) __attribute__ ((format (printf, 1, 2)));
void dbgvprintf(const char *fmt, va_list ap);
void dbgprintf_full(const char *fmt,...) __attribute__ ((format (printf, 1, 2)));
void dbgvprintf_full(const char *fmt, va_list ap);

#endif
