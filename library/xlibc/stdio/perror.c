/*
 * perror.c - print an error message on the standard error output
 */
/* $Header: perror.c,v 1.1 89/05/30 13:31:30 eck Exp $ */

#if	defined(_POSIX_SOURCE)
#include	<sys/types.h>
#endif
#include	<stdio.h>
#include	<errno.h>
#include	<stdio.h>
#include	<string.h>
#include	"loc_incl.h"
#include	<unistd.h>

void
perror(const char *s)
{
	char *p;
	int fd;

	p = strerror(errno);
	fd = fileno(stderr);
	fflush(stdout);
	fflush(stderr);
	if (s && *s) {
		write(fd, (void *)s, strlen(s));
		write(fd, ": ", 2);
	}
	write(fd, p, strlen(p));
	write(fd, "\n", 1);
}
