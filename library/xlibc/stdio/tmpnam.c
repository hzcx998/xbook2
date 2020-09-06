/*
 * tmpnam.c - create a unique filename
 */
/* $Header: tmpnam.c,v 1.4 91/02/26 09:28:39 ceriel Exp $ */

#if	defined(_POSIX_SOURCE)
#include	<sys/types.h>
#endif
#include	<stdio.h>
#include	<string.h>
#include	"loc_incl.h"
#include	<unistd.h>

char *
tmpnam(char *s) {
	static char name_buffer[L_tmpnam] = "/tmp/tmp.";
	static unsigned long count = 0;
	static char *name = NULL;

	if (!name) { 
		name = name_buffer + strlen(name_buffer);
		name = _i_compute((unsigned long)getpid(), 10, name, 5);
		*name++ = '.';
		*name = '\0';
	}
	if (++count > TMP_MAX) count = 1;	/* wrap-around */
	*_i_compute(count, 10, name, 3) = '\0';
	if (s) return strcpy(s, name_buffer);
	else return name_buffer;
}
