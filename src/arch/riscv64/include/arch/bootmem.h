#ifndef _RISCV64_BOOTMEM_H
#define _RISCV64_BOOTMEM_H

#include <stddef.h>

typedef struct {
    unsigned long start_addr;    // 记录从哪个地方开始进行分配
    unsigned long cur_addr;      // 当前分配到的位置的地址
    unsigned long top_addr;      // 引导内存的界限
} boot_mem_t;

void boot_mem_init(unsigned long start, unsigned long size);
void *boot_mem_alloc(size_t size);
unsigned long boot_mem_current_addr();
unsigned long boot_mem_size();
void boot_mem_overview();

#endif  /* _RISCV64_BOOTMEM_H */
