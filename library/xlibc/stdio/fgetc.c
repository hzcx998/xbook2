/*
 * fgetc - get an unsigned character and return it as an int
 */
/* $Header: fgetc.c,v 1.1 89/05/30 13:27:35 eck Exp $ */

#include	<stdio.h>

int
fgetc(FILE *stream)
{
	return getc(stream);
}
