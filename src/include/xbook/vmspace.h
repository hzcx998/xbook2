#ifndef _XBOOK_VMSPACE_H
#define _XBOOK_VMSPACE_H

#include "vmm.h"
#include "memcache.h"

/* 虚拟内存地址空间描述 */
typedef struct vmspace {
    unsigned long start;    /* 空间开始地址 */
    unsigned long end;      /* 空间结束地址 */
    unsigned long prot;     /* 空间保护 */
    unsigned long flags;    /* 空间的标志 */
    vmm_t *vmm;             /* 空间对应的虚拟内存管理 */
    struct vmspace *next;   /* 所有控件构成单向链表 */
} vmspace_t;


/* 虚拟空间的插入。*/
static inline vmspace_insert(vmm_t *vmm, vmspace_t *space)
{
    vmspace_t *prev = NULL, *p = vmm->vmspace_head;
    while (*p) {
        /* 查找在space后面的空间 */
        if (space->end <= p->start)
            break;
        prev = p;
        p = p->next;
    }
    /* p是space的后一个 */
    space->next = p;
    if (prev)   /* 把space插入到prev和p中间 */
        prev->next = space;
    else    /* 如果前一个是空，说明插入到队首 */
        vmm->vmspace_head = space;
    space->vmm = vmm; /* 绑定空间的虚拟内存管理 */
    /* 合并相邻的空间 */
}

static inline vmspace_remove(vmm_t *vmm, vmspace_t *space, vmspace_t *prev)
{
    if (prev)   /* 没在链表头，位于中间或者后面 */
        prev->next = space->next;
    else    /* 前面没有空间，说明在链表头 */
        vmm->vmspace_head = space->next;    
    kfree(space);
}

static inline vmspace_find(vmm_t *vmm, unsigned long addr)
{
    vmspace_t *space = vmm->space_head;
    while (space) {
        if (addr < space->end)  /* 地址小于某个控件结束位置就返回该空间 */
            return space;
        space = space->next;
    }
    return NULL;
}

static inline vmspace_find_prev(vmm_t *vmm, unsigned long addr, vmspace_t *prev)
{
    prev = NULL;    /* prev save prev space ptr, set NULL first */
    vmspace_t *space = vmm->space_head;
    while (space) {
        if (addr < space->end)  /* 地址小于某个控件结束位置就返回该空间 */
            return space;
        prev = space;   /* save prev */
        space = space->next;
    }
    return NULL;
}

/* 如果一个空间的开始地址和另一个空间的结束地址位于同一个空间范围，说明二者相交 */
static inline vmspace_t *vmspace_find_intersection(vmm_t *vmm,
    unsigned long start, unsigned long end)
{
    vmspace_t *space = vmspace_find(vmm, start);
    if (space && space->start >= end) /* 如果第一个空间的开始大于第二个空间的结束，说明不相交 */
        space = NULL;
    return space;
}




#endif /* _XBOOK_VMSPACE_H */
