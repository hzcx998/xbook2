#include <xbook/debug.h>
#include <math.h>
#include "bootmem.h"

static boot_mem_t boot_mem_alloctor;

/* 
 * init_boot_mem - 初始化引导内存分配器
 * 
 * 初始化后就可以进行简单粗暴的内存分配了。
 */
void init_boot_mem(unsigned int start, unsigned int end)
{
    boot_mem_alloctor.start_addr = start;
    boot_mem_alloctor.cur_addr = boot_mem_alloctor.start_addr;
    boot_mem_alloctor.top_addr = end;
}

/*
 * boot_mem_alloc - 分配内存
 * @size: 分配多大的空间
 * 
 * 返回的是一个地址指针
 */
void *boot_mem_alloc(size_t size)
{
    // 获取要分配的地址
    unsigned int addr = boot_mem_alloctor.cur_addr;

    //修改分配地址
    boot_mem_alloctor.cur_addr += size;
    //对地址进行对齐
    boot_mem_alloctor.cur_addr = ALIGN_WITH(boot_mem_alloctor.cur_addr, 32);
    if (boot_mem_alloctor.cur_addr >= boot_mem_alloctor.top_addr) {
        return NULL;
    }
    return (void *)addr;
}

/* 
 * boot_mem_pos - 返回当前分配的地址
 */
unsigned int boot_mem_pos()
{
    return boot_mem_alloctor.cur_addr;
}

/*
 * boot_mem_size - 获取已经分配了多大的空间
 */
unsigned int boot_mem_size()
{
    return boot_mem_alloctor.cur_addr - boot_mem_alloctor.start_addr;
}
