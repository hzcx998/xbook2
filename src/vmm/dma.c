#include <arch/page.h>
#include <arch/memio.h>
#include <xbook/virmem.h>
#include <xbook/dma.h>
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
        vaddr = (addr_t) kern_phy_addr2vir_addr(d->p.address);
    } else { // normal
        d->p.address = page_alloc_user(npages);
        if(d->p.address == 0)
	    	return -1;
        
        vaddr = vir_addr_alloc(d->p.size);
        if (vaddr == 0) {
            page_free(d->p.address);
            return -1;
        }
        if (mem_remap(d->p.address, vaddr, d->p.size) < 0) {
            vir_addr_free(vaddr, d->p.size);
            page_free(d->p.address);
            return -1;
        }
    }
	
	d->v = vaddr;
	return 0;
}

int dma_free_buffer(struct dma_region *d)
{
    /* 参数检测 */
    if (!d->p.size || !d->p.address || !d->v)
        return -1;
    
    if (d->flags & DMA_REGION_SPECIAL) {
        page_free(d->p.address);
    } else {
        if (mem_unmap(d->v, d->p.size) < 0) {
            return -1;
        }

        vir_addr_free(d->v, d->p.size);
        page_free(d->p.address);
    }
    d->p.address = d->v = 0;
	
    return 0;
}
