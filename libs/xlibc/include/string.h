#ifndef _LIB_STRING_H
#define _LIB_STRING_H

#ifdef __cplusplus
extern "C" {
#endif

#include <types.h>
#include <stdint.h>
#include <stddef.h>

char* itoa(char ** ps, int val, int base);
char *itoa16_align(char * str, int num);

void *memset(void* src, uint8_t value, uint32_t size);
void *memcpy(void* _dst, const void* _src, uint32_t size);
int memcmp(const void * s1, const void *s2, int n);
void *memset16(void* src, uint16_t value, uint32_t size);
void *memset32(void* src, uint32_t value, uint32_t size);
void* memmove(void* dst,const void* src,uint32_t count);
void * memchr(const void * s, int c, size_t n);

#define bzero(str, n) memset(str, 0, n) 
#define bcopy(s, d, n) ((void) memcpy ((d), (s), (n)))

char* strcpy(char* _dst, const char* _src);
uint32_t strlen(const char* str);
int8_t strcmp (const char *a, const char *b); 
char *strchr(const char *s,int c);
char* strrchr(const char* str, int c);
char* strcat(char* strDest , const char* strSrc);
int strncmp (const char * s1, const char * s2, int n);
int strpos(char *str, char ch);
char* strncpy(char* _dst, const char* _src, int n) ;
char *strncat(char *dst, const char *src, int n);
int strmet(const char *src, char *buf, char ch);
char *strstr(const char *dest, const char *src);
size_t strspn(const char *s, const char *accept);
const char *strpbrk(const char *str1, const char *str2);
int strcoll(const char *str1, const char *str2);
char * strdup(const char * s);
int strcasecmp(const char * s1, const char * s2);

/*
 * fls - find last (most-significant) bit set
 * Note fls(0) = 0, fls(1) = 1, fls(0x80000000) = 32.
 */
static inline __attribute__((always_inline)) int fls(int x)
{
	return x ? sizeof(x) * 8 - __builtin_clz(x) : 0;
}


#ifdef __cplusplus
}
#endif

#endif  /* _LIB_STDINT_H */

