/*
 * libc/stdlib/strntoimax.c
 */

#include <stdlib.h>

intmax_t strntoimax(const char * nptr, char ** endptr, int base, size_t n)
{
	return (intmax_t)strntoumax(nptr, endptr, base, n);
}
