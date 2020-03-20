#ifndef _XBOOK_MEMOPS_H
#define _XBOOK_MEMOPS_H

#include "stdint.h"
#include "stddef.h"
#include "types.h"

void *memset(void* src, unsigned char value, unsigned int size);
void memcpy(const void* dst, const void* src, uint32_t size);
int memcmp(const void * s1, const void *s2, int n);
void *memset16(void* src, unsigned short value, unsigned int size);
void *memset32(void* src, unsigned int value, unsigned int size);
void* memmove(void* dst,const void* src,unsigned int count);

#define bzero(str, n) memset(str, 0, n) 

#endif  /* _XBOOK_MEMOPS_H */