#ifndef _XBOOK_MEMSPACE_H
#define _XBOOK_MEMSPACE_H

#include "vmm.h"
#include "memcache.h"
#include <stddef.h>
#include <stdint.h>
#include <types.h>
#include <arch/page.h>

#define MEM_SPACE_MAP_FIXED       0x10       /* 映射固定位置 */
#define MEM_SPACE_MAP_STACK       0x20       /* 映射成栈，会动态变化 */
#define MEM_SPACE_MAP_HEAP        0x40       /* 映射成堆，会动态变化 */
#define MEM_SPACE_MAP_SHARED      0x80       /* 映射成共享内存 */
#define MEM_SPACE_MAP_REMAP       0x100      /* 强制重写映射 */

#define MAX_MEM_SPACE_STACK_SIZE  (16 * MB)
#define MEM_SPACE_STACK_SIZE_DEFAULT  (PAGE_SIZE * 4)

#define MAX_MEM_SPACE_HEAP_SIZE    (512 * MB)

#define MEM_SPACE_MAP_ADDR_START  0x60000000
#define MAX_MEM_SPACE_MAP_SIZE    (256 * MB)

typedef struct mem_space {
    unsigned long start;        /* 空间开始地址 */
    unsigned long end;          /* 空间结束地址 */
    unsigned long page_prot;    /* 空间保护 */
    unsigned long flags;        /* 空间的标志 */
    vmm_t *vmm;                 /* 空间对应的虚拟内存管理 */
    struct mem_space *next;     /* 所有空间构成单向链表 */
} mem_space_t;

typedef struct {
    void *addr;
    size_t length;
    int prot;
    int flags;
    int fd;
    off_t offset;
} mmap_args_t;

#define mem_space_alloc() mem_alloc(sizeof(mem_space_t))
#define mem_space_free    mem_free

void mem_space_dump(vmm_t *vmm);
void mem_space_insert(vmm_t *vmm, mem_space_t *space);
int do_mem_space_unmap(vmm_t *vmm, unsigned long addr, unsigned long len);
int do_mem_space_map(vmm_t *vmm, unsigned long addr, unsigned long paddr, 
    unsigned long len, unsigned long prot, unsigned long flags);
void *mem_space_mmap(uint32_t addr, uint32_t paddr, uint32_t len, uint32_t prot,
    uint32_t flags);
int mem_space_unmmap(uint32_t addr, uint32_t len);
unsigned long sys_mem_space_expend_heap(unsigned long heap);
unsigned long mem_space_get_unmaped(vmm_t *vmm, unsigned len);

void *mem_space_mmap_viraddr(uint32_t addr, uint32_t vaddr,
        uint32_t len, uint32_t prot, uint32_t flags);

#define sys_munmap  mem_space_unmmap

static inline void mem_space_init(mem_space_t *space, unsigned long start,
    unsigned long end, unsigned long page_prot, unsigned long flags)
{
    space->start = start;
    space->end = end;
    space->page_prot = page_prot;
    space->flags = flags;
    space->vmm = NULL;
    space->next = NULL;
}

static inline void mem_space_remove(vmm_t *vmm, mem_space_t *space, mem_space_t *prev)
{
    if (prev)
        prev->next = space->next;
    else
        vmm->mem_space_head = space->next;    
    mem_space_free(space);
}

static inline mem_space_t *mem_space_find(vmm_t *vmm, unsigned long addr)
{
    mem_space_t *space = vmm->mem_space_head;
    while (space != NULL) {
        if (addr < space->end)
            return space;
        space = space->next;
    }
    return NULL;
}

static inline mem_space_t *mem_space_find_prev(vmm_t *vmm, unsigned long addr, mem_space_t **prev)
{
    *prev = NULL;    /* prev save prev space ptr, set NULL first */
    mem_space_t *space = vmm->mem_space_head;
    while (space != NULL) {
        if (addr < space->end)
            return space;
        *prev = space;   /* save prev */
        space = space->next;
    }
    return NULL;
}

static inline mem_space_t *mem_space_find_intersection(vmm_t *vmm,
    unsigned long start, unsigned long end)
{
    mem_space_t *space = mem_space_find(vmm, start);
    /* 如果第一个空间的开始大于第二个空间的末尾，说明不相交 */
    if (space && space->start >= end)
        space = NULL;
    return space;
}

#endif /* _XBOOK_MEMSPACE_H */
