#include <stdarg.h>
#include <sizes.h>
#include <stdio.h>

int vfprintf(FILE *f, const char *fmt, va_list ap)
{
    char buf[SZ_4K];
    int rv = vsnprintf(buf, SZ_4K, fmt, ap);
	rv = (fputs(buf, f) < 0) ? 0 : rv;
    return rv;
}