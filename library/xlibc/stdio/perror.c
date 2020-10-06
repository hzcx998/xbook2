/*
 * xlibc/stdio/perror.c
 */

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

void perror(const char *s)
{
	char *p;
	int fd;

	p = strerror(errno);
	fd = stderr->fd;
	fflush(stdout);
	fflush(stderr);
	if (s && *s) {
		write(fd, (void *)s, strlen(s));
		write(fd, ": ", 2);
	}
	write(fd, p, strlen(p));
	write(fd, "\n", 1);
}
