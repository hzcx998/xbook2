#include <k210_qemu_phymem.h>
#include <arch/page.h>
//#include <arch/memory.h>
#include <xbook/debug.h>
#include <math.h>
#include <string.h>
#include <assert.h>

extern unsigned int kernel_end; // first address after kernel.
extern unsigned int kernel_start; // first address after kernel.

static volatile unsigned int total_pmem_size;

unsigned long mem_get_total_page_nr()
{
    return total_pmem_size / PAGE_SIZE;
}

/**
 * 将物理内存划分为3个区域。
 * DMA区域：专门提供给DMA设备使用，在初始化时已经映射到虚拟地址，所以可以直接把物理地址转换成虚拟地址。
 * NORMAL区域：专门提供给内核使用，在初始化时已经映射到虚拟地址，所以可以直接把物理地址转换成虚拟地址。
 * USER区域：专门提供给用户进程空间使用，由于是在使用过程中才映射的，所以不能直接物理地址转虚拟地址。
 *          只能使用计算公式把虚拟地址转物理地址。
 */
int physic_memory_init()
{
    total_pmem_size = PHYSIC_MEM_SIZE;
    printf2("total size:%d %d MB\n", 1, 2);
    keprint("total size:%d %d MB\n", 1, 2);
    
    return 0;
}
