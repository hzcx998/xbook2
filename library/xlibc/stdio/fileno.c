/*
 * fileno .c - map a stream to a file descriptor
 */
/* $Header: fileno.c,v 1.1 89/12/18 14:59:31 eck Exp $ */

#include	<stdio.h>

int
(fileno)(FILE *stream)
{
	return stream->_fd;
}
