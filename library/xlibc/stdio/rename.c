/*
 * libc/stdio/rename.c
 */

#include <stdio.h>
#include <unistd.h>

int rename(const char * old, const char * new)
{
	return _rename(old, new);
}