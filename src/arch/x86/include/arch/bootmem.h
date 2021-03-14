#ifndef _X86_BOOTMEM_H
#define _X86_BOOTMEM_H

#include <stddef.h>

typedef struct {
    unsigned int start_addr;    // 记录从哪个地方开始进行分配
    unsigned int cur_addr;      // 当前分配到的位置的地址
    unsigned int top_addr;      // 引导内存的界限
} boot_mem_t;

void boot_mem_init(unsigned int start, unsigned int size);
void *boot_mem_alloc(size_t size);
unsigned int boot_mem_current_addr();
unsigned int boot_mem_size();

#endif  /* _X86_BOOTMEM_H */
