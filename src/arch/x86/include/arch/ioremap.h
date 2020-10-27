#ifndef _X86_IOREMAP_H
#define _X86_IOREMAP_H

#include <stddef.h>

int phy_addr_remap(unsigned long paddr, unsigned long vaddr, size_t size);
int phy_addr_unmap(unsigned long addr, size_t size);

#endif   /* _X86_IOREMAP_H */
