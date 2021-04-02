#include <xbook/debug.h>
#include <math.h>
#include <arch/bootmem.h>
#include <arch/page.h>

static boot_mem_t boot_mem_alloctor;

void boot_mem_init(unsigned int start, unsigned int size)
{
    boot_mem_alloctor.start_addr = start;
    boot_mem_alloctor.cur_addr = boot_mem_alloctor.start_addr;
    boot_mem_alloctor.top_addr = start + size;
}

void *boot_mem_alloc(size_t size)
{
    size = PAGE_ALIGN(size);
    unsigned int addr = boot_mem_alloctor.cur_addr;
    boot_mem_alloctor.cur_addr += size;
    if (boot_mem_alloctor.cur_addr >= boot_mem_alloctor.top_addr) {
        errprint("boot_mem_alloc: cur=%x top=%x size=%x\n", 
            boot_mem_alloctor.cur_addr, boot_mem_alloctor.top_addr, size);    
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