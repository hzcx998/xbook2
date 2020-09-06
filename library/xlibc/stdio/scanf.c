/*
 * scanf.c - read formatted input from the standard input stream
 */
/* $Header: scanf.c,v 1.1 89/05/30 13:33:03 eck Exp $ */

#include	<stdio.h>
#include	<stdarg.h>
#include	"loc_incl.h"

int
scanf(const char *format, ...)
{
	va_list ap;
	int retval;

	va_start(ap, format);

	retval = _doscan(stdin, format, ap);

	va_end(ap);

	return retval;
}


