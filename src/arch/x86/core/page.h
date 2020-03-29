#ifndef _X86_PAGE_H
#define _X86_PAGE_H

#include "interrupt.h"

// 页目录类型
typedef unsigned int pde_t;
// 页表类型
typedef unsigned int pte_t;

//PDT = PAGE DIR TABLE
//内核的页目录表物理地址
#define PAGE_DIR_PHY_ADDR     0X201000
//内核的页目录表虚拟地址
#define PAGE_DIR_VIR_ADDR     0X80201000

//PT = PAGE TABLE
//内核的页表物理地址
#define PAGE_TABLE_PHY_ADDR     0X202000
//内核的页表虚拟地址
#define PAGE_TABLE_VIR_ADDR     0X80202000

// 在loader中初始化了2个页表，满足需要
#define PAGE_TABLE_PHY_NR     2

#define	PG_P_1	  	1	// 0001 exist in memory
#define	PG_P_0	  	0	// 0000 not exist in memory
#define	PG_RW_R  	0	// 0000 R/W read/execute
#define	PG_RW_W  	2	// 0010 R/W read/write/execute
#define	PG_US_S  	0	// 0000 U/S system level, cpl0,1,2
#define	PG_US_U  	4	// 0100 U/S user level, cpl3

#define PAGE_SHIFT 12

#define PAGE_SIZE (1U<<PAGE_SHIFT)  

#define PAGE_MASK (~(PAGE_SIZE-1))  

#define PAGE_INSIDE (PAGE_SIZE-1)  

//一个页有多少项
#define PAGE_ENTRY_NR 1024  

//获取页目录项的索引(存放了页表的地址和属性)
#define PDE_IDX(addr) ((addr & 0xffc00000) >> 22)

//获取页表项的索引(存放了页的地址和属性)
#define PTE_IDX(addr) ((addr & 0x003ff000) >> 12)

/* 一点内存中页的数量 */
#define PAGE_NR_IN_1GB      (0X40000000 / PAGE_SIZE)
#define PAGE_NR_IN_16MB      (0X1000000 / PAGE_SIZE)

// 保证值和页的大小对齐
#define PAGE_ALIGN(value) ((value + PAGE_SIZE - 1) & PAGE_MASK)

// 检测pte存在
#define PAGE_PTE_EXIST(entry) (entry & PAGE_P_1)

// 通过and运算把一个页地址去掉属性部分
#define PAGE_ADDR_MASK  0xfffff000

/* 页故障导致的错误码 */
#define PG_ERR_NONE_PRESENT       (0<<0)
#define PG_ERR_PROTECT            (1<<0)
#define PG_ERR_READ               (0<<1)
#define PG_ERR_WRITE              (1<<1)
#define PG_ERR_SUPERVISOR         (0<<2)
#define PG_ERR_USER               (1<<2)

static inline pde_t *get_kpdir_ptr()
{
    return (pde_t *)PAGE_DIR_VIR_ADDR;
}

/**
 * get_pde_ptr - 获取pde
 * @vaddr: 虚拟地址
 * 
 * 通过虚拟地址获取它对应的pde
 */
static inline pde_t *get_pde_ptr(unsigned int vaddr)
{
	// 获取地址对应的页目录项地址
	pde_t *pde = (unsigned int *)(0xfffff000 + \
	PDE_IDX(vaddr)*4);
	return pde;
}

/**
 * get_pte_ptr - 获取pte
 * @vaddr: 虚拟地址
 * 
 * 通过虚拟地址获取它对应的pte
 */
static inline pte_t *get_pte_ptr(unsigned int vaddr)
{
	// 获取页表项地址
	pte_t *pte = (unsigned int *)(0xffc00000 + \
	((vaddr & 0xffc00000) >> 10) + PTE_IDX(vaddr)*4);
	return pte;
}

unsigned long __alloc_pages(unsigned long count);
int __free_pages(unsigned long page);

void __page_link(unsigned long va, unsigned long pa, unsigned long prot);
void __page_unlink(unsigned long vaddr);

int __map_pages(unsigned long start, unsigned long len, unsigned long prot);
int __unmap_pages(unsigned long vaddr, unsigned long len);

int __map_pages_safe(unsigned long start, unsigned long len, unsigned long prot);
int __unmap_pages_safe(unsigned long start, unsigned long len);

// 该地址是2GB虚拟地址的偏移
#define PAGE_OFFSET             0X80000000

/* 普通地址转换成物理地址 */
#define __pa(x) ((unsigned long)(x) - PAGE_OFFSET)
/* 普通地址转换成虚拟地址 */ 
#define __va(x) ((void *)((unsigned long)(x) + PAGE_OFFSET)) 

/* 非连续内存地址转换 */
unsigned long __addr_v2p(unsigned long vaddr);

int mem_self_mapping(unsigned int start, unsigned int end);

unsigned long *__copy_kernel_page_dir();

/* active page dir */
#define __page_dir_active(page, on) \
    do { \
        unsigned long paddr = PAGE_DIR_PHY_ADDR; \
        if ((on)) { \
            paddr = page; \
        } \
        write_cr3(paddr); \
        update_tss_info((unsigned long )current_task); \
    } while (0)

void do_page_fault(trap_frame_t *frame);


#endif  /*_X86_MM_PAGE_H */
