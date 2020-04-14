#ifndef _XLIBC_VSPRINTF_H
#define _XLIBC_VSPRINTF_H

#include "stdarg.h"

#define STR_DEFAULT_LEN 256

int vsprintf(char *buf, const char *fmt, va_list args);
int vsnprintf(char *buf, int buflen, const char *fmt, va_list args);
int sprintf(char *buf, const char *fmt, ...);
int snprintf(char *buf, int buflen, const char *fmt, ...);

#endif  /* _XLIBC_VSPRINTF_H */
