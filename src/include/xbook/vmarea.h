#ifndef _XBOOK_VMAREA_H
#define _XBOOK_VMAREA_H

#include <stddef.h>
#include <arch/page.h>

#define VMAREA_BASE     __VMAREA_BASE
#define VMAREA_END      __VMAREA_END

/* 虚拟区域结构体 */
typedef struct vmarea {
	unsigned long addr;
	unsigned long size;
	list_t list;
} vmarea_t;

void *vmalloc(size_t size);
int vfree(void *ptr);

unsigned long alloc_vaddr(size_t size);
unsigned long free_vaddr(unsigned long vaddr, size_t size);

void *ioremap(unsigned long paddr, size_t size);
int iounmap(void *vaddr);


void init_vmarea();

#endif   /* _XBOOK_VMAREA_H */
