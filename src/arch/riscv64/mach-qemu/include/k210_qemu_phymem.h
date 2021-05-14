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

/* 虚拟地址偏移 */
#define VIRT_OFFSET             0x3F00000000L

#ifdef QEMU
// qemu puts UART registers here in physical memory.
#define UART                    0x10000000L
#else
#define UART                    0x38000000L
#endif

#define UART_V                  (UART + VIRT_OFFSET)

// local interrupt controller, which contains the timer.
#define CLINT                   0x02000000L
#define CLINT_V                 (CLINT + VIRT_OFFSET)

// Platform level interrupt controller
#define PLIC                    0x0c000000L
#define PLIC_V                  (PLIC + VIRT_OFFSET)

#define PLIC_PRIORITY           (PLIC_V + 0x0)
#define PLIC_PENDING            (PLIC_V + 0x1000)
#define PLIC_MENABLE(hart)      (PLIC_V + 0x2000 + (hart) * 0x100)
#define PLIC_SENABLE(hart)      (PLIC_V + 0x2080 + (hart) * 0x100)
#define PLIC_MPRIORITY(hart)    (PLIC_V + 0x200000 + (hart) * 0x2000)
#define PLIC_SPRIORITY(hart)    (PLIC_V + 0x201000 + (hart) * 0x2000)
#define PLIC_MCLAIM(hart)       (PLIC_V + 0x200004 + (hart) * 0x2000)
#define PLIC_SCLAIM(hart)       (PLIC_V + 0x201004 + (hart) * 0x2000)


int physic_memory_init();

unsigned long mem_get_free_page_nr();
unsigned long mem_get_total_page_nr();

#endif   /*_K210_QEMU_PHYMEM_H */
