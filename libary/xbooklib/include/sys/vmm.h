#ifndef _SYS_VMM_H
#define _SYS_VMM_H

#include <stddef.h>

/* vmm */
void *heap(void *heap);
int munmap(void *addr, size_t length);

#endif  /* _SYS_VMM_H */
