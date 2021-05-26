#ifndef _K210_QEMU_PHYMEM_H
#define _K210_QEMU_PHYMEM_H

#include <const.h>

// 0MB~2MB是SBI的内存分布
#define RUSTSBI_MEM_ADDR                0x80000000
#ifndef QEMU
#define RUSTSBI_MEM_SIZE                0x20000    
#else
#define RUSTSBI_MEM_SIZE                0x200000    // 2MB
#endif

// 内核的内存地址
#define KERN_MEM_ADDR                   (RUSTSBI_MEM_ADDR + RUSTSBI_MEM_SIZE)

/* boot mem的大小可以根据内存使用情况进行调整，获取最优大小。 */
#ifndef QEMU
#define BOOT_MEM_SIZE                   0x20000     // 128KB
#else
#define BOOT_MEM_SIZE                   0x10000     // 64KB
#endif

/* 物理内存最高地址 */
#define PHYSIC_MEM_SIZE                 0x600000    // 6MB
#define PHYSIC_MEM_TOP                  (RUSTSBI_MEM_ADDR + PHYSIC_MEM_SIZE)

/* 内核虚拟地址偏移，内核的所有 */
#define VIRT_OFFSET             0x3F00000000L

// one beyond the highest possible virtual address.
// MAXVA is actually one bit less than the max allowed by
// Sv39, to avoid having to sign-extend virtual addresses
// that have the high bit set.
#define MAX_VIR_ADDR (1L << (9 + 9 + 9 + 12 - 1))

// map the trampoline page to the highest address,
// in both user and kernel space.
#define TRAMPOLINE              (MAX_VIR_ADDR - PAGE_SIZE)

// User memory layout.
// Address zero first:
//   text
//   original data and bss
//   fixed-size stack
//   expandable heap
//   ...
//   TRAPFRAME (p->trapframe, used by the trampoline)
//   TRAMPOLINE (the same page as in the kernel)
#define TRAPFRAME               (TRAMPOLINE - PAGE_SIZE)

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

/* 除了PLIC_PENDING和PLIC_MENABLE、PLIC_SENABLE的单位是以位为单位外，其他的都是以字节为一个单位 */
#define PLIC_PRIORITY           (PLIC_V + 0x0)      // 以4字节为单位
#define PLIC_PENDING            (PLIC_V + 0x1000)   // 以位为单位
#define PLIC_MENABLE(hart)      (PLIC_V + 0x2000 + (hart) * 0x100)      // machine interrupt enable，以位为单位
#define PLIC_SENABLE(hart)      (PLIC_V + 0x2080 + (hart) * 0x100)      // supervisor interrupt enable，以位为单位
#define PLIC_MPRIORITY(hart)    (PLIC_V + 0x200000 + (hart) * 0x2000)   // 优先级值，以4字节为单位
#define PLIC_SPRIORITY(hart)    (PLIC_V + 0x201000 + (hart) * 0x2000)   // 以4字节为单位
#define PLIC_MCLAIM(hart)       (PLIC_V + 0x200004 + (hart) * 0x2000)   // 以4字节为单位
#define PLIC_SCLAIM(hart)       (PLIC_V + 0x201004 + (hart) * 0x2000)   // 以4字节为单位


int physic_memory_init();

unsigned long mem_get_free_page_nr();
unsigned long mem_get_total_page_nr();

#endif   /*_K210_QEMU_PHYMEM_H */
