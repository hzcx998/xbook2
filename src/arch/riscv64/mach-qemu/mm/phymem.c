#include <k210_qemu_phymem.h>
#include <arch/page.h>
#include <arch/bootmem.h>
#include <arch/mempool.h>
//#include <arch/memory.h>
#include <xbook/debug.h>
#include <xbook/task.h>
#include <math.h>
#include <string.h>
#include <assert.h>

extern char kernel_end[]; // final address after kernel.
extern char kernel_start[]; // first address after kernel.
extern char boot_stack[];   // boot stack

/* extern from task/task.c */

static volatile unsigned long total_pmem_size;

unsigned long mem_get_total_page_nr()
{
    return total_pmem_size / PAGE_SIZE;
}

int physic_memory_init()
{
    total_pmem_size = PHYSIC_MEM_SIZE;
    u64_t kern_size = (u64_t) (kernel_end - kernel_start);
    u64_t free_size = total_pmem_size - kern_size - RUSTSBI_MEM_SIZE;
    keprint("memory total size:%#x Bytes %d MB\n", total_pmem_size, total_pmem_size / MB);
    keprint("kernel image: [%#p, %#p]\n", kernel_start, kernel_end);
    keprint("kernel size: %#x Bytes %d MB\n", kern_size, kern_size / MB);
    keprint("free size: %#x Bytes %d MB\n", free_size, free_size / MB);
    
    u64_t boot_mem_start = KERN_MEM_ADDR + kern_size;
    u64_t boot_mem_sz = BOOT_MEM_SIZE;
    
    boot_mem_init(boot_mem_start, boot_mem_sz);
    /* 只初始化normal范围内存 */
    mem_range_init(MEM_RANGE_NORMAL, boot_mem_start + boot_mem_sz, free_size - boot_mem_sz);

    boot_mem_overview();

    kernel_stack_buttom = boot_stack;

    // mem_pool_test();
    return 0;
}
