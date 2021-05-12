#include <xbook/debug.h>
#include <arch/page.h>
#include <arch/memio.h>

/**
 * 因为memio是直接映射一个物理地址，因此不需要分配物理页，直接使用指定的页地址即可，释放同理
 */
int hal_memio_remap(unsigned long paddr, unsigned long vaddr, size_t size)
{
    return mappages(GET_CUR_PGDIR(), vaddr, size, paddr, PAGE_ATTR_READ | PAGE_ATTR_WRITE | PAGE_ATTR_SYSTEM);
}

int hal_memio_unmap(unsigned long addr, size_t size)
{
    vmunmap(GET_CUR_PGDIR(), addr, size / PAGE_SIZE, 0);
    return 0;
}