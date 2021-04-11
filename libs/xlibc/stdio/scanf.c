/*
 * xlibc/stdio/scanf.c
 */

#include <sizes.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>

int scanf(const char * fmt, ...)
{
	va_list ap;
	char * buf;
	int rv;

	buf = malloc(SZ_4K);
	if(!buf)
		return 0;

	memset(buf, 0, SZ_4K);
	fread(buf, 1, SZ_4K, stdin);

	va_start(ap, fmt);
	rv = vsscanf(buf, fmt, ap);
	va_end(ap);

	free(buf);
	return rv;
}
