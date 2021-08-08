#ifndef _LIB_STRING_H
#define _LIB_STRING_H

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>
#include <stdint.h>
#include <stddef.h>

void *memset(void* src, unsigned char value, size_t size);
void *memcpy(void* _dst, const void* _src, size_t size);
int memcmp(const void * s1, const void *s2, int n);
void *memset16(void* src, unsigned short value, size_t size);
void *memset32(void* src, unsigned int value, size_t size);
void* memmove(void* dst,const void* src, size_t count);
void * memchr(const void * s, int c, size_t n);

#define bzero(str, n) memset(str, 0, n) 
#define bcopy(s, d, n) ((void) memcpy ((d), (s), (n)))

char* strcpy(char* _dst, const char* _src);
int strlen(const char* str);
int strcmp (const char *a, const char *b); 
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

char *strchrnul(const char *, int);

#if defined(__X86__)
/*
 * fls - find last (most-significant) bit set
 * Note fls(0) = 0, fls(1) = 1, fls(0x80000000) = 32.
 */
static inline __attribute__((always_inline)) int fls(int x)
{
	return x ? sizeof(x) * 8 - __builtin_clz(x) : 0;
}
#else 
static inline __attribute__((always_inline)) int fls(int x)
{
    int r = 32;

    if (!x)
        return 0;
    if (!(x & 0xffff0000u)) {
        x <<= 16;
        r -= 16;
    }
    if (!(x & 0xff000000u)) {
        x <<= 8;
        r -= 8;
    }
    if (!(x & 0xf0000000u)) {
        x <<= 4;
        r -= 4;
    }
    if (!(x & 0xc0000000u)) {
        x <<= 2;
        r -= 2;
    }
    if (!(x & 0x80000000u)) {
        x <<= 1;
        r -= 1;
    }
    return r;
}
#endif

#ifdef __cplusplus
}
#endif

#endif  /*_LIB_STDINT_H*/

