/*
 * libc/stdlib/strtof.c
 */

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>

/*
 * Convert string to float
 */
float strtof(const char * nptr, char ** endptr)
{
	return (float)strtod(nptr, endptr);
}
