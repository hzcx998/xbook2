#ifndef _X86_IOREMAP_H
#define _X86_IOREMAP_H

#include <xbook/stddef.h>

int __ioremap(unsigned long paddr, unsigned long vaddr, size_t size);
int __iounmap(unsigned long addr, size_t size);

#endif   /* _X86_IOREMAP_H */
