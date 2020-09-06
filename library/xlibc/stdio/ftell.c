/*
 * ftell.c - obtain the value of the file-position indicator of a stream
 */
/* $Header: ftell.c,v 1.4 90/01/22 11:12:12 eck Exp $ */

#include	<stdio.h>

#if	(SEEK_CUR != 1) || (SEEK_SET != 0) || (SEEK_END != 2)
#error SEEK_* values are wrong
#endif

#include	"loc_incl.h"

#include	<sys/types.h>
#include	<unistd.h>

long ftell(FILE *stream)
{
	long result;
	int adjust = 0;

	if (io_testflag(stream,_IOREADING))
		adjust = -stream->_count;
	else if (io_testflag(stream,_IOWRITING)
		    && stream->_buf
		    && !io_testflag(stream,_IONBF))
		adjust = stream->_ptr - stream->_buf;
	else adjust = 0;

	result = lseek(fileno(stream), (off_t)0, SEEK_CUR);

	if ( result == -1 )
		return result;

	result += (long) adjust;
	return result;
}
