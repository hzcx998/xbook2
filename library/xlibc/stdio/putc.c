/*
 * xlibc/stdio/putc.c
 */

#include <stdio.h>

int putc(int c, FILE * f)
{
	return fputc(c, f);
}
