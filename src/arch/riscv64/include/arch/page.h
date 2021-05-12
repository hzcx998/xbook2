#ifndef _RISCV64_PAGE_H
#define _RISCV64_PAGE_H

#include <xbook/config.h>
#include "mempool.h"
#include <stdint.h>

typedef uint64_t pde_t; /* page dir entry */
typedef uint64_t pte_t; /* page table entry */
typedef uint64_t *pgdir_t; // 512 PTEs

/* 内核空间在映射后的虚拟基地址 */
#define KERN_BASE_VIR_ADDR      0X80000000

#define	PAGE_ATTR_PRESENT	  	(1L << 0) // valid
#define	PAGE_ATTR_READ  	    (1L << 1)
#define	PAGE_ATTR_WRITE  	    (1L << 2)
#define	PAGE_ATTR_EXEC  	    (1L << 3)
#define	PAGE_ATTR_USER  	    (1L << 4)   // 1 -> user can access
#define	PAGE_ATTR_SYSTEM  	    (0)

#define KERN_PAGE_ATTR  (PAGE_ATTR_PRESENT | PAGE_ATTR_WRITE | PAGE_ATTR_SYSTEM)

#define PAGE_SHIFT  12
#define PAGE_SIZE   (1U << PAGE_SHIFT)  
#define PAGE_LIMIT  (PAGE_SIZE-1)
#define PAGE_MASK   (~PAGE_LIMIT)  
#define PAGE_ALIGN(value) ((value + PAGE_LIMIT) & PAGE_MASK)

#define PAGE_TABLE_ENTRY_NR 512  

// shift a physical address to the right place for a PTE.
#define PA2PTE(pa) ((((uint64)pa) >> 12) << 10)

#define PTE2PA(pte) (((pte) >> 10) << 12)

#define PTE_FLAGS(pte) ((pte) & 0x3FF)

// extract the three 9-bit page table indices from a virtual address.
#define PXMASK          0x1FF // 9 bits
#define PXSHIFT(level)  (PAGE_SHIFT+(9*(level)))
#define PX(level, va) ((((uint64) (va)) >> PXSHIFT(level)) & PXMASK)

// one beyond the highest possible virtual address.
// MAXVA is actually one bit less than the max allowed by
// Sv39, to avoid having to sign-extend virtual addresses
// that have the high bit set.
#define MAX_VIR_ADDR (1L << (9 + 9 + 9 + 12 - 1))

#define VIRT_OFFSET             0x3F00000000L

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

/* protect flags */
#define PROT_NONE        0x0       /* page can not be accessed */
#define PROT_READ        0x1       /* page can be read */
#define PROT_WRITE       0x2       /* page can be written */
#define PROT_EXEC        0x4       /* page can be executed */
#define PROT_KERN        0x8       /* page in kernel */
#define PROT_USER        0x10      /* page in user */
#define PROT_REMAP       0x20      /* page remap */

/* only have MEM_NODE_TYPE_NORMAL */
#define page_alloc_normal(count)            mem_node_alloc_pages(count, MEM_NODE_TYPE_NORMAL)
#define page_alloc_user(count)              mem_node_alloc_pages(count, MEM_NODE_TYPE_NORMAL)
#define page_alloc_dma(count)               mem_node_alloc_pages(count, MEM_NODE_TYPE_NORMAL)
#define page_free(addr)                     mem_node_free_pages(addr)

unsigned long *kern_page_dir_copy_to();
#define kern_page_copy_storge               kern_page_dir_copy_to

#endif  /* _RISCV64_PAGE_H */
