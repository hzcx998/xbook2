/*
 * xlibc/stdio/fgets.c
 */

#include <stddef.h>
#include <stdio.h>

char * fgets(char * s, int n, FILE * f)
{
	char * p = s;
	char * ret = NULL;
	ssize_t res = 0;

	while(n-- > 1)
	{
		res = __stdio_read(f, (unsigned char *)p, 1);

		if(res == 0)
			break;

		else if(res < 0)
			return NULL;

		ret = s;
		/* add \b */
        if(*p == '\b') {
            *p = '\0';
            if (p > s) {
                --p;
                *p = '\0';
            }
        } else if(*p++ == '\n')
			break;
	}

	*p = 0;
	return ret;
}
