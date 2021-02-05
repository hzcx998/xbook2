#include <xbook/debug.h>
#include <math.h>
#include <arch/bootmem.h>

static boot_mem_t boot_mem_alloctor;

void boot_mem_init(unsigned int start, unsigned int end)
{
    boot_mem_alloctor.start_addr = start;
    boot_mem_alloctor.cur_addr = boot_mem_alloctor.start_addr;
    boot_mem_alloctor.top_addr = end;
}

void *boot_mem_alloc(size_t size)
{
    unsigned int addr = boot_mem_alloctor.cur_addr;
    boot_mem_alloctor.cur_addr += size;
    boot_mem_alloctor.cur_addr = ALIGN_WITH(boot_mem_alloctor.cur_addr, 32);
    if (boot_mem_alloctor.cur_addr >= boot_mem_alloctor.top_addr) {
        return NULL;
    }
    return (void *)addr;
}

unsigned int boot_mem_current_addr()
{
    return boot_mem_alloctor.cur_addr;
}

unsigned int boot_mem_size()
{
    return boot_mem_alloctor.cur_addr - boot_mem_alloctor.start_addr;
}