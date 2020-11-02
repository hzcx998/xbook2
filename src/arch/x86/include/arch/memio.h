#ifndef _X86_MEMIO_H
#define _X86_MEMIO_H

#include <stddef.h>

int mem_remap(unsigned long paddr, unsigned long vaddr, size_t size);
int mem_unmap(unsigned long addr, size_t size);

#endif   /* _X86_MEMIO_H */
