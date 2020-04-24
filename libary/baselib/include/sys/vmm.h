#ifndef _SYS_VMM_H
#define _SYS_VMM_H

#include <stddef.h>

/* vmm */
void *heap(void *heap);

void *malloc(size_t size);
void free(void *ptr);


#endif  /* _SYS_VMM_H */
