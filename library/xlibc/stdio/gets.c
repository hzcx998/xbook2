/*
 * gets.c - read a line from a stream
 */
/* $Header: gets.c,v 1.2 89/12/18 15:03:00 eck Exp $ */

#include	<stdio.h>

char *
gets(char *s)
{
	register FILE *stream = stdin;
	register int ch;
	register char *ptr;

	ptr = s;
	while ((ch = getc(stream)) != EOF && ch != '\n')
		*ptr++ = ch;

	if (ch == EOF) {
		if (feof(stream)) {
			if (ptr == s) return NULL;
		} else return NULL;
	}

	*ptr = '\0';
	return s;
}
