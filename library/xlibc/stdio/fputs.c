/*
 * fputs - print a string
 */
/* $Header: fputs.c,v 1.2 89/12/18 15:02:01 eck Exp $ */

#include	<stdio.h>

int
fputs(register const char *s, register FILE *stream)
{
	register int i = 0;

	while (*s) 
		if (putc(*s++, stream) == EOF) return EOF;
		else i++;

	return i;
}
