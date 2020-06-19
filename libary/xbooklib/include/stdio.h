#ifndef _LIB_STDIO_H
#define _LIB_STDIO_H

#include "stdarg.h"


#define STR_DEFAULT_LEN 1024


#define EOF -1

int printf(const char *fmt, ...);

int vsprintf(char *buf, const char *fmt, va_list args);
int vsnprintf(char *buf, int buflen, const char *fmt, va_list args);
int sprintf(char *buf, const char *fmt, ...);
int snprintf(char *buf, int buflen, const char *fmt, ...);

int getchar(void);
int putchar(int ch);

/* temp function, do nothing  */
int fflush(void *file);

#endif  /* _LIB_STDIO_H */
