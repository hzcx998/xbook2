/*
 * xlibc/stdio/rename.c
 */

#include <stdio.h>
#include <unistd.h>

int rename(const char * old, const char * _new)
{
	return _rename(old, _new);
}