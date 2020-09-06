/*
 * puts.c - print a string onto the standard output stream
 */
/* $Header: puts.c,v 1.2 89/12/18 15:03:30 eck Exp $ */

#include	<stdio.h>

int
puts(register const char *s)
{
	register FILE *file = stdout;
	register int i = 0;

	while (*s) {
		if (putc(*s++, file) == EOF) return EOF;
		else i++;
	}
	if (putc('\n', file) == EOF) return EOF;
	return i + 1;
}
