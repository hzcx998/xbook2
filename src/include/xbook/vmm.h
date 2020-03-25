#ifndef _XBOOK_VMM_H
#define _XBOOK_VMM_H

#include <arch/page.h>
#include "vmspace.h"
/* 进程空间虚拟内存管理 */
typedef struct vmm {
    void *page_storage;                     /* 虚拟内存管理的结构 */                   
    vmspace_t *vmspace_head;                /* 虚拟空间头 */

    unsigned long code_start, code_end;     /* 代码空间范围 */
    unsigned long data_start, data_end;     /* 数据空间范围 */
    unsigned long bss_start, bss_end;       /* 未初始化数据空间范围 */
    unsigned long heap_start, heap_end;     /* 堆空间范围 */
    unsigned long map_start, map_end;       /* 映射空间范围 */
    unsigned long share_start, share_end;   /* 映射空间范围 */
    unsigned long stack_start, stack_end;   /* 栈空间范围 */
    unsigned long arg_start, arg_end;       /* 参数空间范围 */
    unsigned long env_start, env_end;       /* 环境空间范围 */
} vmm_t;

#define vmm_alloc() (vmm_t *)kmalloc(sizeof(vmm_t)); 

void vmm_init(vmm_t *vmm);

/* 如果vmm为空，那么就加载内核页，如果不为空，就夹在vmm的页 */
#define vmm_active(vmm) \
    page_dir_active(v2p((vmm)->page_storage), (vmm) == NULL ? 0 : 1)

#endif  /* _XBOOK_VMM_H */
