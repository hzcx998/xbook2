/*
 * xlibc/stdio/tmpname.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

char * tmpnam(char * buf)
{
	static char internal[L_tmpnam];
	struct stat st;
	char path[MAX_PATH];

	do {
		sprintf(path, "%s/tmpnam_%d", "/tmp", rand());
	} while(stat(path, &st) < 0);

	return strcpy(buf ? buf : internal, path);
}