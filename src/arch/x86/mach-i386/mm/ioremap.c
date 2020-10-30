#include <xbook/debug.h>
#include <arch/page.h>
#include <arch/ioremap.h>

int mem_remap(unsigned long paddr, unsigned long vaddr, size_t size)
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

int mem_unmap(unsigned long addr, size_t size)
{
    unsigned long end = addr + size;
    
    /* 取消虚拟地址的内存映射 */
    while (addr < end) {
        page_unlink_addr(addr);
        addr += PAGE_SIZE;
    }
    return 0;
}