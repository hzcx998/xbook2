#ifndef _XBOOK_STRING_H
#define _XBOOK_STRING_H

#include <stdint.h>
#include <stddef.h>
#include <types.h>

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

/* 字符的最大长度 */
#define  STRING_MAX_LEN  256

typedef struct _string {
    unsigned int length;
    unsigned int max_length;
    char *text;
} string_t;

void string_init(string_t *string);
int string_new(string_t *string, char *text, unsigned int maxlen);
void string_copy(string_t *string, char *text);
void string_empty(string_t *string);
void string_del(string_t *string);

void *memset(void* src, unsigned char value, unsigned int size);
void memcpy(const void* dst, const void* src, uint32_t size);
int memcmp(const void * s1, const void *s2, int n);
void *memset16(void* src, unsigned short value, unsigned int size);
void *memset32(void* src, unsigned int value, unsigned int size);
void* memmove(void* dst,const void* src,unsigned int count);

#define bzero(str, n) memset(str, 0, n) 

#endif  /* _XBOOK_STRING_H */

