#include <xbook/debug.h>
#include <arch/page.h>
#include <arch/memio.h>

/**
 * 因为memio是直接映射一个物理地址，因此不需要分配物理页，直接使用指定的页地址即可，释放同理
 */

int hal_memio_remap(unsigned long paddr, unsigned long vaddr, size_t size)
{
    unsigned long end = vaddr + size;
    while (vaddr < end) {
        /* 添加页面 */
        page_link_addr(vaddr, paddr, PAGE_ATTR_WRITE | PAGE_ATTR_SYSTEM);
        vaddr += PAGE_SIZE;
        paddr += PAGE_SIZE;
    }
    return 0;
}

int hal_memio_unmap(unsigned long addr, size_t size)
{
    unsigned long end = addr + size;
    
    /* 取消虚拟地址的内存映射 */
    while (addr < end) {
        page_unlink_addr(addr);
        addr += PAGE_SIZE;
    }
    return 0;
}