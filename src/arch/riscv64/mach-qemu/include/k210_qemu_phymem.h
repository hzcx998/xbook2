#ifndef _K210_QEMU_PHYMEM_H
#define _K210_QEMU_PHYMEM_H

#include <const.h>

// 0MB~2MB是SBI的内存分布
#define RUSTSBI_MEM_ADDR                0x80000000
#define RUSTSBI_MEM_SIZE                0x200000    // 2MB
// 内核的内存地址
#define KERN_MEM_ADDR                   (RUSTSBI_MEM_ADDR + RUSTSBI_MEM_SIZE)

/* boot mem的大小可以根据内存使用情况进行调整，获取最优大小。 */
#define BOOT_MEM_SIZE                   0x10000     // 64KB

/* 物理内存最高地址 */
#define PHYSIC_MEM_SIZE                 0x600000    // 6MB
#define PHYSIC_MEM_TOP                  (RUSTSBI_MEM_ADDR + PHYSIC_MEM_SIZE)

int physic_memory_init();

unsigned long mem_get_free_page_nr();
unsigned long mem_get_total_page_nr();

#endif   /*_K210_QEMU_PHYMEM_H */
