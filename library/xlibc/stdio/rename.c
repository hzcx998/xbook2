/*
 * remove.c - remove a file
 */
/* $Header: remove.c,v 1.2 90/01/22 11:12:44 eck Exp $ */

#include	<stdio.h>
#include	<unistd.h>

int
rename(const char *_old, const char *_new) {
	return _rename(_old, _new);
}
