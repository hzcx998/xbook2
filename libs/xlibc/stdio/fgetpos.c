/*
 * xlibc/stdio/fgetpos.c
 */

#include <stdio.h>

int fgetpos(FILE * f, fpos_t * pos)
{
	*pos = f->pos;
	return 0;
}
