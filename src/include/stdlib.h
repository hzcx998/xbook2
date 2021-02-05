#ifndef _LIB_STDLIB_H
#define _LIB_STDLIB_H

#include "stddef.h"
#include "types.h"
#include <xbook/memalloc.h>

void abort(void);

#define malloc  mem_alloc
#define free    mem_free

#define RAND_MAX 0x7fff

void srand(unsigned long seed);
int rand();

#endif  /* _LIB_STDLIB_H */
