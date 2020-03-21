#include <xbook/debug.h>
#include "page.h"
#include "ioremap.h"

int __ioremap(unsigned long paddr, unsigned long vaddr, size_t size)
{
    unsigned long end = vaddr + size;
    while (vaddr < end) {
        /* 添加页面 */
        __page_link((void *)vaddr, (void *)paddr, PG_RW_W | PG_US_S);
        vaddr += PAGE_SIZE;
        paddr += PAGE_SIZE;
    }
    return 0;
}

int __iounmap(unsigned long addr, size_t size)
{
    unsigned long end = addr + size;
    
    /* 取消虚拟地址的内存映射 */
    while (addr < end) {
        __page_unlink(addr);
        addr += PAGE_SIZE;
    }
    return 0;
}