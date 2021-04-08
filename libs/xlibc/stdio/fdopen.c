/*
 * xlibc/stdio/fdopen.c
 */

#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

FILE * fdopen(int fildes, const char * mode)
{
    if (fildes < 0 || !mode)
        return NULL;
    FILE * f;
    int flags = O_RDONLY;
	int plus = 0;
    
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

	f = __file_alloc(fildes);
	if(!f) {
		return NULL;
	}
    fcntl(fildes, F_SETFL, &flags);
	f->pos = lseek(f->fd, 0, SEEK_SET);
    return f;
}