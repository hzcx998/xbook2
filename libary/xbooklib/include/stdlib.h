#ifndef _LIB_STDLIB_H
#define _LIB_STDLIB_H

#include "stddef.h"
#include "types.h"
#include <sys/proc.h>

#define RAND_MAX 0x7fff

void abort(void);


void srand(unsigned long seed);
int rand();


void free(void *ptr);
void *malloc(size_t size);
void memory_state(void);
int malloc_usable_size(void *ptr);
void *calloc(size_t count, size_t size);
void *realloc(void *ptr, size_t size);

#define exit    _exit


#endif  /* _LIB_STDLIB_H */
