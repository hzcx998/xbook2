/*
 * setbuf.c - control buffering of a stream
 */
/* $Header: setbuf.c,v 1.2 89/06/26 10:36:22 eck Exp $ */

#include	<stdio.h>
#include	"loc_incl.h"

void
setbuf(register FILE *stream, char *buf)
{
	(void) setvbuf(stream, buf, (buf ? _IOFBF : _IONBF), (size_t) BUFSIZ);
}
