/*
 * libc/stdio/remove.c
 */

#include <stdio.h>
#include <unistd.h>

int remove(const char * path)
{
	return unlink(path);
}
