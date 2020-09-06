/*
 * fscanf.c - read formatted input from stream
 */
/* $Header: fscanf.c,v 1.1 89/05/30 13:29:17 eck Exp $ */

#include	<stdio.h>
#include	<stdarg.h>
#include	"loc_incl.h"

int
fscanf(FILE *stream, const char *format, ...)
{
	va_list ap;
	int retval;

	va_start(ap, format);

	retval = _doscan(stream, format, ap);

	va_end(ap);

	return retval;
}
