#include <arch/page.h>
#include <arch/ioremap.h>
#include <xbook/vmarea.h>
#include <xbook/dma.h>
#include <math.h>

int alloc_dma_buffer(struct dma_region *d)
{
    /* 参数检测 */
    if (!d->p.size)
        return -1;
    
    int npages = DIV_ROUND_UP(d->p.size, PAGE_SIZE);
    addr_t vaddr;
    if (d->flags & DMA_REGION_SPECIAL) {
        d->p.address = alloc_dma_pages(npages);
        if(d->p.address == 0)
            return -1;
        vaddr = (addr_t) p2v(d->p.address);
    } else { // normal
        d->p.address = alloc_pages(npages);
        if(d->p.address == 0)
	    	return -1;
        
        vaddr = alloc_vaddr(d->p.size);
        if (vaddr == 0) {
            free_pages(d->p.address);
            return -1;
        }
        if (__ioremap(d->p.address, vaddr, d->p.size) < 0) {
            free_vaddr(vaddr, d->p.size);
            free_pages(d->p.address);
            return -1;
        }
    }
	
	d->v = vaddr;
	return 0;
}

int free_dma_buffer(struct dma_region *d)
{
    /* 参数检测 */
    if (!d->p.size || !d->p.address || !d->v)
        return -1;
    
	if (__iounmap(d->v, d->p.size) < 0) {
        return -1;
    }

    free_vaddr(d->v, d->p.size);
    free_pages(d->p.address);

	d->p.address = d->v = 0;
	return 0;
}
