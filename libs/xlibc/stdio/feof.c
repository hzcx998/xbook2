/*
 * xlibc/stdio/feof.c
 */

#include <stdio.h>

int feof(FILE * f)
{
	return f->eof;
}
