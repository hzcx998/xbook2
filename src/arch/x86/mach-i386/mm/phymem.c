#include <arch/phymem.h>
#include <arch/mempool.h>
#include <arch/page.h>
#include <arch/bootmem.h>
#include <arch/memory.h>
#include <xbook/debug.h>
#include <xbook/task.h>
#include <math.h>
#include <string.h>
#include <assert.h>

static volatile unsigned long total_pmem_size;

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
    total_pmem_size = phy_mem_get_size_from_hardware();
    assert(total_pmem_size >= (300 * MB));
    /* 根据内存大小划分区域
    如果内存大于1GB:
        1G预留128MB给非连续内存，其余给内核和用户程序平分，内核多余的部分分给用户
    如果内存小于1GB：
        预留128MB给非连续内存，其余平分给内核和用户程序
     */
    unsigned int normal_size;
    unsigned int user_size;
    
    unsigned int unused_size;
    
    unused_size = KERN_BLACKHOLE_MEM_SIZE + NORMAL_MEM_ADDR;
    
    normal_size = (total_pmem_size - unused_size) / 2; 
    user_size = total_pmem_size - unused_size - normal_size;
    
    if (normal_size > 1*GB) {
        unsigned int more_size = normal_size - 1*GB;
        user_size += more_size;
        normal_size -= more_size;
    }
    
    noteprint("total size:%x %d MB\n", total_pmem_size, total_pmem_size / MB);
    noteprint("normal size:%x %d MB\n", normal_size, normal_size / MB);
    noteprint("user size:%x %d MB\n", user_size, user_size / MB);
    noteprint("unused size:%x %d MB\n", unused_size, unused_size / MB);
    
    /* 由于引导中只映射了0~8MB，所以这里从DMA开始 */
    kern_page_map_early(DMA_MEM_ADDR, NORMAL_MEM_ADDR + normal_size);

    /* normal size前面是boot mem，后面是normal mem */
    boot_mem_init(KERN_BASE_VIR_ADDR + BOOT_MEM_ADDR, BOOT_MEM_SIZE);
    mem_range_init(MEM_RANGE_DMA, DMA_MEM_ADDR, DMA_MEM_SIZE);
    mem_range_init(MEM_RANGE_NORMAL, BOOT_MEM_ADDR + BOOT_MEM_SIZE, normal_size - BOOT_MEM_SIZE);
    mem_range_init(MEM_RANGE_USER, NORMAL_MEM_ADDR + normal_size, user_size - KERN_BLACKHOLE_MEM_SIZE);

    kernel_stack_buttom = (char *)KERNEL_STATCK_BOTTOM;
    // mem_pool_test();
    
    // spin("memcheck");
    return 0;
}   
