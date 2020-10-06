/*
 * xlibc/stdio/getchar.c
 */

#include <stdio.h>

int getchar(void)
{
	return fgetc(stdin);
}
