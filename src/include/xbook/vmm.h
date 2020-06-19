#ifndef _XBOOK_VMM_H
#define _XBOOK_VMM_H

#include <arch/page.h>

#define USER_VMM_SIZE       PAGE_OFFSET
#define USER_STACK_TOP      USER_VMM_SIZE
#define VMM_UNMAPPED_BASE    (USER_VMM_SIZE / 2)

/* 进程空间虚拟内存管理 */
typedef struct vmm {
    void *page_storage;                     /* 虚拟内存管理的结构 */                   
    void *vmspace_head;                     /* 虚拟空间头,设置成空类型，使用时转换类型 */

    unsigned long code_start, code_end;     /* 代码空间范围 */
    unsigned long data_start, data_end;     /* 数据空间范围 */
    unsigned long heap_start, heap_end;     /* 堆空间范围 */
    unsigned long map_start, map_end;       /* 映射空间范围 */
    unsigned long share_start, share_end;   /* 共享空间范围 */
    unsigned long stack_start, stack_end;   /* 栈空间范围 */
    unsigned long arg_start, arg_end;       /* 参数空间范围 */
    unsigned long env_start, env_end;       /* 环境空间范围 */
} vmm_t;

/* 物理内存信息 */
typedef struct {
    unsigned long ms_total;    /* 物理内存总大小 */
    unsigned long ms_free;     /* 物理内存空闲大小 */
    unsigned long ms_used;     /* 物理内存已使用大小 */
} mstate_t;

int sys_mstate(mstate_t *ms);


#define vmm_alloc() (vmm_t *)kmalloc(sizeof(vmm_t)) 
#define vmm_free(vmm) kfree(vmm) 

void vmm_init(vmm_t *vmm);
int vmm_exit(vmm_t *vmm);
int vmm_release_space(vmm_t *vmm);
int vmm_unmap_space(vmm_t *vmm);

void dump_vmm(vmm_t *vmm);

void vmm_active(vmm_t *vmm);

#endif  /* _XBOOK_VMM_H */
