#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <sconf.h>

char *sconf_readline(char *buf, const char *line, int len)
{
    if (!buf || !line || len <= 0)
        return NULL;
    if (*buf == '\0')
        return NULL;
    char *p = buf;
    char *q = (char *) line;
    while (*p) {
        if (*p == '\r') {
            p++;
            if (*p == '\n') {   /* '\r\n' */
                p++;
                break;
            } else {    /* '\r' */
                break;
            }
        }
        if (*p == '\n') { /* '\n' */
            p++;
            break;
        }
        if (len > 0) {
            *q++ = *p;
            len--;
        }
        p++;
    }
    q = (char *) line;
    q[len - 1] = '\0';
    return p;
}

char *sconf_read(char *line, const char *str, int len)
{
    if (!line || !str)
        return NULL;
    if (*line == '\0')
        return NULL;

    char *p = line;
    char *q = (char *) str;

    while (*p) {
        if (*p == SCONF_SEPARATOR) {
            p++;
            break;
        }
        if (len > 0) {
            *q++ = *p;
            len--;
        }
        p++;
    }

    q = (char *) str;
    q[len - 1] = '\0';
    return p;
}

int sconf_writeline(char *line)
{
    if (!line)
        return -1;
    char sbuf[2] = {'\n',0};
    strcat(line, (const char *) sbuf);
    return 0;
}

int sconf_write(char *line, const char *str)
{
    if (!line || !str)
        return -1;
    strcat(line, str);
    const char sbuf[2] = {SCONF_SEPARATOR, '\0'};
    strcat(line, sbuf);
    return 0;
}

char *sconf_trim(const char *str)
{
    if (!str)
        return NULL;

    char *start, *end;
	int len = strlen(str);
	start = (char *) str;
	end = start + len -1;
	while (*start && isspace(*start))
		start++;
    while (*end && isspace(*end))
		*end-- = 0;

    len = strlen(start);
    strcpy((char *) str, start);
    start = (char *) str;
    start[len] = '\0';
    return start;
}

int sconf_int(const char *str)
{
    if (!str)
        return 0;
    return atoi(str);
}

int sconf_bool(const char *str)
{
    if (!str)
        return 0;
    if (!strncmp(str, "true", 4))
        return 1;
    return 0;
}