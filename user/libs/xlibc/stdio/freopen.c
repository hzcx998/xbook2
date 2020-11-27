/*
 * xlibc/stdio/freopen.c
 */

#include <stdio.h>

FILE * freopen(const char * path, const char * mode, FILE * f)
{
	/* Not support redirect yet */
	if(f)
		fclose(f);
	return fopen(path, mode);
}
