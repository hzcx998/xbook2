/*
 * libc/stdio/tmpfile.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

FILE * tmpfile(void)
{
	struct stat st;
	char path[MAX_PATH];

	do {
		sprintf(path, "%s/tmpfile_%d", "/tmp", rand());
	} while(stat(path, &st) < 0);

	return fopen(path, "wb+");
}
