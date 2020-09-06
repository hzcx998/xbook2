/*
 * vfprintf - formatted output without ellipsis
 */
/* $Header: vfprintf.c,v 1.2 89/12/18 15:04:07 eck Exp $ */

#include	<stdio.h>
#include	<stdarg.h>
#include	"loc_incl.h"

int
vfprintf(FILE *stream, const char *format, va_list arg)
{
	return _doprnt (format, arg, stream);
}
