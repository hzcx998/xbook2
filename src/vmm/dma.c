#include <arch/page.h>
#include <arch/memio.h>
#include <xbook/virmem.h>
#include <xbook/dma.h>
#include <xbook/debug.h>
#include <math.h>

int dma_alloc_buffer(struct dma_region *d)
{
    /* 参数检测 */
    if (!d->p.size)
        return -1;
    
    int npages = DIV_ROUND_UP(d->p.size, PAGE_SIZE);
    addr_t vaddr;
    if (d->flags & DMA_REGION_SPECIAL) {
        d->p.address = page_alloc_dma(npages);
        if(d->p.address == 0)
            return -1;
    } else { // normal
        d->p.address = page_alloc_normal(npages);
        if(d->p.address == 0)
	    	return -1;
    }
    vaddr = (addr_t) kern_phy_addr2vir_addr(d->p.address);
	
	d->v = vaddr;
    return 0;
}

int dma_free_buffer(struct dma_region *d)
{
    /* 参数检测 */
    if (!d->p.size || !d->p.address || !d->v)
        return -1;
    
    page_free(d->p.address);
    
    d->p.address = d->v = 0;
	
    return 0;
}

void dma_buffer_dump(struct dma_region *d)
{
    keprint("dma vir addr:%x, phy addr:%x, size:%x, flags:%x\n", d->v, d->p.address, d->p.size, d->flags);
}