/*
 * fputc.c - print an unsigned character
 */
/* $Header: fputc.c,v 1.1 89/05/30 13:28:45 eck Exp $ */

#include	<stdio.h>

int
fputc(int c, FILE *stream)
{
	return putc(c, stream);
}
