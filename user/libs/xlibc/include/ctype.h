
#ifndef _LIB_CTYPE_H
#define _LIB_CTYPE_H
#ifdef __cplusplus
extern "C" {
#endif
int isspace(char c);
int isalnum(int ch);
int isxdigit (int c);
int isdigit( int ch );
int isalpha(int ch);
int tolower(int c);
int toupper(int c);
int isdigitstr(const char *str);
int isgraph(int ch);
int islower (int ch);
int iscntrl(int ch);
int isupper(int ch);
int ispunct(int ch);
int isprint( int ch );
#define isascii(c) (((unsigned) c)<=0x7f)

#define isblank(c) ((c) == ' ' || (c) == '\t')

#ifdef __cplusplus
}
#endif

#endif  /* _LIB_CTYPE_H */
