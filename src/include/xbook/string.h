#ifndef _XBOOK_STRING_H
#define _XBOOK_STRING_H

#include "stdint.h"
#include "stddef.h"
#include "types.h"

char* itoa(char ** ps, int val, int base);
int atoi(const char *src);
char *itoa16_align(char * str, int num);
int strncmp (const char * s1, const char * s2, int n);
unsigned int strlen(const char* str);
char strcmp (const char *a, const char *b); 
char *strchr(const char *s,int c);
char* strrchr(const char* str, int c);
char* strcat(char* strDest , const char* strSrc);
int strncmp (const char * s1, const char * s2, int n);
int strpos(char *str, char ch);
char* strncpy(char* dst, char* src, int n) ;
char *strncat(char *dst, const char *src, int n);
int strmet(const char *src, char *buf, char ch);
char *strstr(const char *dest, const char *src);
size_t strspn(const char *s, const char *accept);
const char *strpbrk(const char *str1, const char *str2);
int strcoll(const char *str1, const char *str2);
char* strcpy(char* dst, const char* src);

#endif  /* _XBOOK_STRING_H */

