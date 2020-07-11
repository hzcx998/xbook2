
#ifndef _XLIBC_MALLOC_H
#define _XLIBC_MALLOC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

void free(void *ptr);
void *calloc(int num, size_t size);
void *malloc(size_t size);
void *realloc(void *oldp, size_t size);
void *memalign (size_t boundary, size_t size);

#ifdef __cplusplus
}
#endif

#endif  /* _XLIBC_MALLOC_H */