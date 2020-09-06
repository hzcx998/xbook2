/*
 * fgetpos.c - get the position in the file
 */
/* $Header: fgetpos.c,v 1.2 89/12/18 15:01:03 eck Exp $ */

#include	<stdio.h>

int
fgetpos(FILE *stream, fpos_t *pos)
{
	*pos = ftell(stream);
	if (*pos == -1) return -1;
	return 0;
}
