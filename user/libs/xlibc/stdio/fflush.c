/*
 * xlibc/stdio/fflush.c
 */

#include <errno.h>
#include <stdio.h>

int fflush(FILE * f)
{
    if (!f)
        return EINVAL;
	if(!f->write)
		return EINVAL;
	return __stdio_write_flush(f);
}
