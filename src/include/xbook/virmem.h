#ifndef _XBOOK_VIR_MEM_H
#define _XBOOK_VIR_MEM_H

#include <stddef.h>
#include <xbook/list.h>
#include <types.h>
#include <arch/page.h>
#include <arch/phymem.h>

#define VIR_MEM_BASE     DYNAMIC_MAP_MEM_ADDR
#define VIR_MEM_END      DYNAMIC_MAP_MEM_END

typedef struct {
	unsigned long addr;
	unsigned long size;
	list_t list;
} vir_mem_t;

void *vir_mem_alloc(size_t size);
int vir_mem_free(void *ptr);

unsigned long vir_addr_alloc(size_t size);
unsigned long vir_addr_free(unsigned long vaddr, size_t size);

void *memio_remap(unsigned long paddr, size_t size);
int memio_unmap(void *vaddr);

void vir_mem_init();

#endif   /* _XBOOK_VIR_MEM_H */
