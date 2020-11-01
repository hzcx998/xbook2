#ifndef _X86_PAGE_H
#define _X86_PAGE_H

#include "interrupt.h"
#include "mempool.h"

typedef unsigned int pde_t; /* page dir entry */
typedef unsigned int pte_t; /* page table entry */

/* 内核空间在映射后的虚拟基地址 */
#define KERN_BASE_VIR_ADDR      0X80000000

#define KERN_PAGE_DIR_PHY_ADDR       0X201000
#define KERN_PAGE_DIR_VIR_ADDR       (KERN_BASE_VIR_ADDR + KERN_PAGE_DIR_PHY_ADDR)

#define KERN_PAGE_TABLE_PHY_ADDR     0X202000
#define KERN_PAGE_TABLE_VIR_ADDR     (KERN_BASE_VIR_ADDR + KERN_PAGE_TABLE_PHY_ADDR)

/* 在引导阶段就已经使用过的页表数，映射内核空间时就不再对最开始的页表进行映射 */
#define PAGE_TABLE_HAD_USED     2

#define	PAGE_ATTR_NOT_PRESENT	0	// 0000 not exist in memory
#define	PAGE_ATTR_PRESENT	  	1	// 0001 exist in memory
#define	PAGE_ATTR_READ  	    0	// 0000 R/W read/execute
#define	PAGE_ATTR_WRITE  	    2	// 0010 R/W read/write/execute
#define	PAGE_ATTR_SYSTEM  	    0	// 0000 U/S system level, cpl0,1,2
#define	PAGE_ATTR_USER  	    4   // 0100 U/S user level, cpl3

#define KERN_PAGE_ATTR  (PAGE_ATTR_PRESENT | PAGE_ATTR_WRITE | PAGE_ATTR_SYSTEM)


#define PAGE_SHIFT  12
#define PAGE_SIZE   (1U << PAGE_SHIFT)  
#define PAGE_LIMIT  (PAGE_SIZE-1)
#define PAGE_MASK   (~PAGE_LIMIT)  
#define PAGE_ALIGN(value) ((value + PAGE_LIMIT) & PAGE_MASK)

#define PAGE_TABLE_ENTRY_NR 1024  
#define KERN_PAGE_DIR_ENTRY_OFF 512  

#define PAGE_DIR_ENTRY_IDX(addr)    ((addr & 0xffc00000) >> 22)
#define PAGE_TABLE_ENTRY_IDX(addr)  ((addr & 0x003ff000) >> 12)

#define PAGE_ERR_NONE_PRESENT       (0<<0)
#define PAGE_ERR_PROTECT            (1<<0)
#define PAGE_ERR_READ               (0<<1)
#define PAGE_ERR_WRITE              (1<<1)
#define PAGE_ERR_SUPERVISOR         (0<<2)
#define PAGE_ERR_USER               (1<<2)

static inline pde_t *vir_addr_to_dir_entry(unsigned int vaddr)
{
	pde_t *pde = (unsigned int *)(0xfffff000 + \
	PAGE_DIR_ENTRY_IDX(vaddr)*4);
	return pde;
}

static inline pte_t *vir_addr_to_table_entry(unsigned int vaddr)
{
	pte_t *pte = (unsigned int *)(0xffc00000 + \
	((vaddr & 0xffc00000) >> 10) + PAGE_TABLE_ENTRY_IDX(vaddr)*4);
	return pte;
}

void page_link_addr(unsigned long va, unsigned long pa, unsigned long prot);
void page_unlink_addr(unsigned long vaddr);

int page_map_addr(unsigned long start, unsigned long len, unsigned long prot);
int page_unmap_addr(unsigned long vaddr, unsigned long len);

int page_map_addr_safe(unsigned long start, unsigned long len, unsigned long prot);
int page_unmap_addr_safe(unsigned long start, unsigned long len, char fixed);

int page_map_addr_fixed(unsigned long start, unsigned long addr, 
    unsigned long len, unsigned long prot);

#define kern_vir_addr2phy_addr(x) ((unsigned long)(x) - KERN_BASE_VIR_ADDR)
#define kern_phy_addr2vir_addr(x) ((void *)((unsigned long)(x) + KERN_BASE_VIR_ADDR)) 

unsigned long addr_vir2phy(unsigned long vaddr);

void kern_page_map_early(unsigned int start, unsigned int end);
unsigned long *kern_page_dir_copy_to();

int page_do_fault(trap_frame_t *frame);

/* protect flags */
#define PROT_NONE        0x0       /* page can not be accessed */
#define PROT_READ        0x1       /* page can be read */
#define PROT_WRITE       0x2       /* page can be written */
#define PROT_EXEC        0x4       /* page can be executed */
#define PROT_KERN        0x8       /* page in kernel */
#define PROT_USER        0x10      /* page in user */
#define PROT_REMAP       0x20      /* page remap */

#define page_alloc_normal(count)            mem_node_alloc_pages(count, MEM_NODE_TYPE_NORMAL)
#define page_alloc_dma(count)               mem_node_alloc_pages(count, MEM_NODE_TYPE_DMA)

#define page_free(addr)                     mem_node_free_pages(addr)

#define page_alloc_one()                    page_alloc_normal(1)
#define page_free_one(addr)                 mem_node_free_pages(addr)

#define kern_page_copy_storge               kern_page_dir_copy_to

#endif  /* _X86_PAGE_H */
