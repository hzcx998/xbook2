#ifndef _LIBC_STDIO_H__
#define _LIBC_STDIO_H__

#include <types.h>
#include <stdarg.h>
#include <stddef.h>
#include <errno.h>
#include <limits.h>
#include <xbook/debug.h>

#ifndef EOF
#define EOF			(-1)
#endif

#ifndef BUFSIZ
#define BUFSIZ		(4096)
#endif

/*
 * Stdio file position type
 */
typedef loff_t fpos_t;

/*
 * stdio state variables.
 */
typedef struct __FILE FILE;
struct __FILE {
	int fd;
};

int fflush(FILE * f);

#define printf  keprint

#define STR_DEFAULT_LEN 256

int vsprintf(char *buf, const char *fmt, va_list args);
int vsnprintf(char *buf, int buflen, const char *fmt, va_list args);
int sprintf(char *buf, const char *fmt, ...);
int snprintf(char *buf, int buflen, const char *fmt, ...);

#endif /* _LIBC_STDIO_H__ */
