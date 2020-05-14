#ifndef _LIB_STDIO_H
#define _LIB_STDIO_H

#include "stdarg.h"

int printf(const char *fmt, ...);

#define STR_DEFAULT_LEN 256

int vsprintf(char *buf, const char *fmt, va_list args);
int vsnprintf(char *buf, int buflen, const char *fmt, va_list args);
int sprintf(char *buf, const char *fmt, ...);
int snprintf(char *buf, int buflen, const char *fmt, ...);

/* temp function, do nothing  */
int fflush(void *file);

#endif  /* _LIB_STDIO_H */
