#ifndef SCONF_H_INCLUDED
#define SCONF_H_INCLUDED

/*
string configure (sconf)
based on: strcat,isspace,strcpy,stren,strncmp,atoi
*/

char *sconf_readline(char *buf, const char *line, int len);
char *sconf_read(char *line, const char *str, int len);

char *sconf_trim(const char *str);
int sconf_writeline(char *line);
int sconf_write(char *line, const char *str);

int sconf_bool(const char *str);
int sconf_int(const char *str);
char *sconf_trim(const char *str);

char sconf_get_separator(void);
void sconf_set_separator(char separator);

#endif // SCONF_H_INCLUDED
