/*
 * xlibc/stdio/fopen.c
 */

#include <errno.h>
#include <stdio.h>
#include <unistd.h>

FILE * fopen(const char * path, const char * mode)
{
	FILE * f;
	int flags = O_RDONLY;
	int plus = 0;
	int fd;

	if((path == NULL) || (*path == '\0'))
	{
		errno = EINVAL;
		return NULL;
	}

	while(*mode)
	{
		switch(*mode++)
		{
		case 'r':
			flags = O_RDONLY;
			break;
		case 'w':
			flags = O_WRONLY | O_CREAT | O_TRUNC;
			break;
		case 'a':
			flags = O_WRONLY | O_CREAT | O_APPEND;
			break;
        case 'b':
			flags |= O_BINARY;
			break;
		case '+':
			plus = 1;
			break;
		}
	}

	if(plus)
		flags = (flags & ~(O_RDONLY | O_WRONLY)) | O_RDWR;

	fd = open(path, flags);
	if(fd < 0)
		return NULL;

	f = __file_alloc(fd);
	if(!f)
	{
		close(fd);
		return NULL;
	}
	f->pos = lseek(f->fd, 0, SEEK_SET);

	return f;
}