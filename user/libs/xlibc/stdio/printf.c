/*
 * xlibc/stdio/printf.c
 */

#include <stdarg.h>
#include <sizes.h>
#include <stdio.h>

int printf(const char * fmt, ...)
{
	va_list ap;
	char buf[SZ_1K] = {0};
	int rv;

	va_start(ap, fmt);
	rv = vsnprintf(buf, SZ_1K, fmt, ap);
	va_end(ap);

	rv = (fputs(buf, stdout) < 0) ? 0 : rv;
	fflush(stdout);
	return rv;
}
