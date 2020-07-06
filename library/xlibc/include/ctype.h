
#ifndef _LIB_CTYPE_H
#define _LIB_CTYPE_H

int isspace(char c);
int isalnum(int ch);
int isxdigit (int c);
int isdigit( int ch );
unsigned long strtoul(const char *cp,char **endp,unsigned int base);
long strtol(const char *cp,char **endp,unsigned int base);
int isalpha(int ch);
double strtod(const char* s, char** endptr);
double atof(char *str);
int tolower(int c);
int toupper(int c);
int isdigitstr(const char *str);
int isgraph(int ch);
int islower (int ch);
int iscntrl(int ch);
int isupper(int ch);
int ispunct(int ch);
int isprint( int ch );

#endif  /* _LIB_CTYPE_H */
