/*
 * xlibc/stdio/ftell.c
 */

#include <errno.h>
#include <stdio.h>

fpos_t ftell(FILE * f)
{
	return f->pos;
}
