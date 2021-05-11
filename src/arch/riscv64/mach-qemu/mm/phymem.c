#include <k210_qemu_phymem.h>
#include <arch/page.h>
//#include <arch/memory.h>
#include <xbook/debug.h>
#include <math.h>
#include <string.h>
#include <assert.h>

extern char kernel_end[]; // first address after kernel.
extern char kernel_start[]; // first address after kernel.

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
    keprint("memory total size:0x%x Bytes %d MB\n", total_pmem_size, total_pmem_size / MB);
    keprint("kernel image: [0x%p, 0x%p]\n", kernel_start, kernel_end);
    keprint("kernel size: 0x%x Bytes %d MB\n", kern_size, kern_size / MB);
    keprint("free size: 0x%x Bytes %d MB\n", free_size, free_size / MB);
    return 0;
}
