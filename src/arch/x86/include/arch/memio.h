#ifndef _X86_MEMIO_H
#define _X86_MEMIO_H

#include <stddef.h>

int hal_memio_remap(unsigned long paddr, unsigned long vaddr, size_t size);
int hal_memio_unmap(unsigned long addr, size_t size);

#endif   /* _X86_MEMIO_H */
