#ifndef _XBOOK_VMM_DMA_H
#define _XBOOK_VMM_DMA_H

#include <types.h>

struct mm_physical_region {
	addr_t address;
	size_t size;
	addr_t alignment;
};

struct dma_region {
	struct mm_physical_region p;
	addr_t v;
};

int alloc_dma_buffer(struct dma_region *);
int free_dma_buffer(struct dma_region *);

#endif   /* _XBOOK_VMM_DMA_H */
