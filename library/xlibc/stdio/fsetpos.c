/*
 * fsetpos.c - set the position in the file
 */
/* $Header: fsetpos.c,v 1.1 89/05/30 13:29:34 eck Exp $ */

#include	<stdio.h>

int
fsetpos(FILE *stream, fpos_t *pos)
{
	return fseek(stream, *pos, SEEK_SET);
}
