#ifndef _XBOOK_VMM_H
#define _XBOOK_VMM_H

#include <arch/page.h>
#include <arch/phymem.h>

#define VMM_NO_SHM

#define USER_VMM_SIZE       USER_SPACE_SIZE
#define USER_VMM_BASE_ADDR  USER_SPACE_START_ADDR
#define USER_VMM_TOP_ADDR   (USER_VMM_BASE_ADDR + USER_VMM_SIZE)
#define USER_STACK_TOP      (USER_VMM_TOP_ADDR - PAGE_SIZE)

/* 进程空间虚拟内存管理 */
typedef struct vmm {
    void *page_storage;                     /* 虚拟内存管理的结构 */                   
    void *mem_space_head;                     /* 虚拟空间头,设置成空类型，使用时转换类型 */
    char **envp;    /* 环境变量指针 */     
    char **argv;    /* 参数变量 */
    char *argbuf;   /* 参数的缓冲区首地址 */
    unsigned long code_start, code_end;     /* 代码空间范围 */
    unsigned long data_start, data_end;     /* 数据空间范围 */
    unsigned long heap_start, heap_end;     /* 堆空间范围 */
    unsigned long map_start, map_end;       /* 映射空间范围 */
    unsigned long stack_start, stack_end;   /* 栈空间范围 */
} vmm_t;

/* 物理内存信息 */
typedef struct {
    unsigned long ms_total;    /* 物理内存总大小 */
    unsigned long ms_free;     /* 物理内存空闲大小 */
    unsigned long ms_used;     /* 物理内存已使用大小 */
} mstate_t;

void vmm_init(vmm_t *vmm);
int vmm_exit(vmm_t *vmm);
void vmm_free(vmm_t *vmm);
int vmm_exit_when_fork_failed(vmm_t *vmm, vmm_t *parent_vmm);
void vmm_free_storage(vmm_t *vmm);
int vmm_release_space(vmm_t *vmm);
int vmm_unmap_space(vmm_t *vmm);
int vmm_unmap_the_mapping_space(vmm_t *vmm);
void vmm_dump(vmm_t *vmm);
int vmm_copy_mem_space(vmm_t *child_vmm, vmm_t *parent_vmm);
void vmm_active(vmm_t *vmm);
int vmm_build_argbug(vmm_t *vmm, char **argv, char **envp);
void vmm_debuild_argbuf(vmm_t *vmm);

int sys_mstate(mstate_t *ms);

void memory_overview();

#endif  /* _XBOOK_VMM_H */
