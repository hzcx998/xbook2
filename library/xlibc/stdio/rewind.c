/*
 * rewind.c - set the file position indicator of a stream to the start
 */
/* $Header: rewind.c,v 1.1 89/05/30 13:32:52 eck Exp $ */

#include	<stdio.h>
#include	"loc_incl.h"

void
rewind(FILE *stream)
{
	(void) fseek(stream, 0L, SEEK_SET);
	clearerr(stream);
}
