#include <arch/phymem.h>
#include <arch/mempool.h>
#include <arch/page.h>
#include <arch/bootmem.h>
#include <arch/memory.h>
#include <xbook/debug.h>
#include <math.h>
#include <string.h>

static unsigned long total_pmem_size;

unsigned long mem_get_total_page_nr()
{
    return total_pmem_size / PAGE_SIZE;
}

static void cut_used_mem()
{
    unsigned int used_mem = boot_mem_size();
    unsigned int used_pages = DIV_ROUND_UP(used_mem, PAGE_SIZE);
    printk("phymem: cut used pages %d\n", used_pages);
    page_alloc_normal(used_pages);
}

int physic_memory_init()
{
    total_pmem_size = phy_mem_get_size_from_hardware();
    /* 根据内存大小划分区域
    如果内存大于1GB:
        1G预留128MB给非连续内存，其余给内核和用户程序平分，内核多余的部分分给用户
    如果内存小于1GB：
        预留128MB给非连续内存，其余平分给内核和用户程序
     */
    unsigned int normal_size;
    unsigned int user_size;
    
    normal_size = (total_pmem_size - 
        (NORMAL_MEM_ADDR + DYNAMIC_MAP_MEM_SIZE + KERN_BLACKHOLE_MEM_SIZE)) / 2; 
    user_size = total_pmem_size - normal_size - NORMAL_MEM_ADDR;
    
    if (normal_size > 1*GB) {
        unsigned int more_size = normal_size - 1*GB;
        user_size += more_size;
        normal_size -= more_size;
    }
    
    /* 由于引导中只映射了0~8MB，所以这里从DMA开始 */
    kern_page_map_early(DMA_MEM_ADDR, NORMAL_MEM_ADDR + normal_size);
    boot_mem_init(KERN_BASE_VIR_ADDR + NORMAL_MEM_ADDR , KERN_BASE_VIR_ADDR + (NORMAL_MEM_ADDR + normal_size));
    
    mem_range_init(MEM_RANGE_DMA, DMA_MEM_ADDR, DMA_MEM_SIZE);
    mem_range_init(MEM_RANGE_NORMAL, NORMAL_MEM_ADDR, normal_size);
    mem_range_init(MEM_RANGE_USER, NORMAL_MEM_ADDR + normal_size, user_size);
    //mem_pool_test();
    
    cut_used_mem();
    return 0;
}   
