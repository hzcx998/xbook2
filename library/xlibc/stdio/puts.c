/*
 * libc/stdio/puts.c
 */

#include <stdio.h>

int puts(const char *str)
{
	fputs(str, stdout);
	fputc('\n', stdout); // 末尾加回车
	return 0;
}
