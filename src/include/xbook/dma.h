#ifndef _XBOOK_VMM_DMA_H
#define _XBOOK_VMM_DMA_H

#include <types.h>

#define DMA_REGION_SPECIAL 0x01    // special addr

struct mm_physical_region {
	addr_t address;
	size_t size;
	addr_t alignment;
};

struct dma_region {
	struct mm_physical_region p;
	addr_t v;
    flags_t flags;
};

int dma_alloc_buffer(struct dma_region *);
int dma_free_buffer(struct dma_region *);

#endif   /* _XBOOK_VMM_DMA_H */
