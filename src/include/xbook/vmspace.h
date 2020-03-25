#ifndef _XBOOK_VMSPACE_H
#define _XBOOK_VMSPACE_H

#include "vmm.h"

/* 虚拟内存地址空间描述 */
typedef struct vmspace {
    unsigned long start;    /* 空间开始地址 */
    unsigned long end;      /* 空间结束地址 */
    unsigned long prot;     /* 空间保护 */
    unsigned long flags;    /* 空间的标志 */
    vmm_t *vmm;             /* 空间对应的虚拟内存管理 */
    struct vmspace *next;   /* 所有控件构成单向链表 */
} vmspace_t;


/* 虚拟空间的插入。
推演：
space 0 100
space 200 300
space 100 200
 */
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



#endif /* _XBOOK_VMSPACE_H */
