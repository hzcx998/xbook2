#ifndef _LIB_STDLIB_H
#define _LIB_STDLIB_H

#include "stddef.h"
#include "types.h"

#define RAND_MAX 0x7fff

void abort(void);


void srand(unsigned long seed);
int rand();

#endif  /* _LIB_STDLIB_H */
