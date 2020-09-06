/*
 * fclose.c - flush a stream and close the file
 */
/* $Header: fclose.c,v 1.4 90/01/22 11:10:54 eck Exp $ */

#include	<stdio.h>
#include	<stdlib.h>
#include	"loc_incl.h"
#include	<unistd.h>

int
fclose(FILE *fp)
{
	register int i, retval = 0;

	for (i=0; i<FOPEN_MAX; i++)
		if (fp == __iotab[i]) {
			__iotab[i] = 0;
			break;
		}
	if (i >= FOPEN_MAX)
		return EOF;
	if (fflush(fp)) retval = EOF;
	if (close(fileno(fp))) retval = EOF;
	if ( io_testflag(fp,_IOMYBUF) && fp->_buf )
		free((void *)fp->_buf);
	if (fp != stdin && fp != stdout && fp != stderr)
		free((void *)fp);
	return retval;
}
