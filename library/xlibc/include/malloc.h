
#ifndef _XLIBC_MALLOC_H
#define _XLIBC_MALLOC_H

#include <stddef.h>
#if 0
void free(void *ptr);
void *malloc(size_t size);
void memory_state(void);
int malloc_usable_size(void *ptr);
void *calloc(size_t count, size_t size);
void *realloc(void *ptr, size_t size);

/* 默认是4字节对齐 */
#define memalign(a, b) malloc(b)

#else
void free(void *p);
void *calloc(int n,size_t size);
void *malloc(size_t size);
void *realloc(void *p,size_t size);
void *memalign (size_t boundary, size_t size);
//#define memalign(a, b) malloc(b)
size_t getmsz(void *ptr);
int setmsz(void *ptr, size_t size);
#endif

#endif  /* _XLIBC_MALLOC_H */