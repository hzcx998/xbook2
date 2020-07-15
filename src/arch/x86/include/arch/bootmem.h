#ifndef _X86_BOOTMEM_H
#define _X86_BOOTMEM_H

#include <stddef.h>

/* 
bootmem是用于初始化内核内存空间以及其数据结构的一个简单
分配器。在这里，我们尽量做得简单。
*/
typedef struct boot_mem 
{
    unsigned int start_addr;   // 记录从哪个地方开始进行分配
    unsigned int cur_addr;     // 当前分配到的位置的地址
    unsigned int top_addr;  // 引导内存的界限
} boot_mem_t;

void init_boot_mem(unsigned int start, unsigned int limit);
void *boot_mem_alloc(size_t size);
unsigned int boot_mem_pos();
unsigned int boot_mem_size();

#endif  /* _X86_BOOTMEM_H */
