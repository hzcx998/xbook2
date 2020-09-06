/*
 * vprintf - formatted output without ellipsis to the standard output stream
 */
/* $Header: vprintf.c,v 1.3 89/12/18 15:04:14 eck Exp $ */

#include	<stdio.h>
#include	<stdarg.h>
#include	"loc_incl.h"

int
vprintf(const char *format, va_list arg)
{
	return _doprnt(format, arg, stdout);
}
