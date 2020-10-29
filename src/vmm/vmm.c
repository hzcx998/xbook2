#include <arch/pmem.h>
#include <arch/vmm.h>
#include <xbook/vmm.h>
#include <xbook/debug.h>
#include <xbook/vmspace.h>

void vmm_init(vmm_t *vmm)
{
    vmm->page_storage = kern_page_copy_storge();
    if (vmm->page_storage == NULL) {
        panic(KERN_EMERG "task_init_vmm: kmalloc for page_storege failed!\n");
    }
    vmm->vmspace_head = NULL;
    /* 其它参数 */
    
}

int sys_mstate(mstate_t *ms)
{
    ms->ms_total    = mem_get_total_page_nr() * PAGE_SIZE;
    ms->ms_free     = mem_get_free_page_nr() * PAGE_SIZE;
    ms->ms_used     = ms->ms_total - ms->ms_free;
    if (ms->ms_used < 0)
        ms->ms_used = 0;
    return 0;
}

void dump_vmm(vmm_t *vmm)
{
    printk(KERN_DEBUG "code: start=%x, end=%x\n", vmm->code_start, vmm->code_end);
    printk(KERN_DEBUG "data: start=%x, end=%x\n", vmm->data_start, vmm->data_end);
    printk(KERN_DEBUG "heap: start=%x, end=%x\n", vmm->heap_start, vmm->heap_end);
    printk(KERN_DEBUG "map: start=%x, end=%x\n", vmm->map_start, vmm->map_end);
    printk(KERN_DEBUG "stack: start=%x, end=%x\n", vmm->stack_start, vmm->stack_end);
}


void vmm_active(vmm_t *vmm)
{
    if (vmm == NULL) {
        vmm_active_kernel();
    } else {   
        vmm_active_user(kern_vir_addr2phy_addr(vmm->page_storage));
    }
}

/**
 * vmm_release_space - 释放掉进程空间管理
 * @vmm: 虚拟内存管理
 * 
 * 以及释放对应的空间
 * 额外需要释放共享空间
 * 
 * @return: 成功返回0， 失败返回-1
 */
int vmm_release_space(vmm_t *vmm)
{
    if (vmm == NULL)
        return -1; 
    /* 释放虚拟空间地址描述 */
    vmspace_t *space = (vmspace_t *)vmm->vmspace_head;

    vmspace_t *p;
    while (space != NULL) {
        p = space;
        space = space->next;
        vmspace_free(p); /* 释放空间 */
    }
    vmm->vmspace_head = NULL;

    vmm->code_start = 0;
    vmm->code_end = 0;
    vmm->data_start = 0;
    vmm->data_end = 0;
    vmm->heap_start = 0;
    vmm->heap_end = 0;
    vmm->map_start = 0;
    vmm->map_end = 0;
    vmm->stack_start = 0;
    vmm->stack_end = 0;
    return 0;
}

/**
 * vmm_unmap_space - 取消虚拟空间映射
 * @vmm: 虚拟内存管理
 * 
 * 取消虚拟地址映射
 * 额外需要释放共享空间
 * 
 * @return: 成功返回0， 失败返回-1
 */
int vmm_unmap_space(vmm_t *vmm)
{
    if (vmm == NULL)
        return -1; 
    /* 释放虚拟空间地址描述 */
    vmspace_t *space = (vmspace_t *)vmm->vmspace_head;

    /* 取消虚拟空间的地址映射 */
    while (space != NULL) {
        /* 由于内存区域可能不是连续的，所以需要用安全的方式来取消映射 */
        page_unmap_addr_safe(space->start, space->end - space->start, space->flags & VMS_MAP_SHARED);
        space = space->next;
    }
    return 0;
}

/**
 * vmm_unmap_space_maparea - 释放映射区域
 * @vmm: 虚拟内存管理
 * 
 * 取消虚拟区域地址映射
 * 
 * @return: 成功返回0， 失败返回-1
 */
int vmm_unmap_space_maparea(vmm_t *vmm)
{
    if (vmm == NULL)
        return -1; 
    /* 释放虚拟空间地址描述 */
    vmspace_t *space = (vmspace_t *)vmm->vmspace_head;

    /* 取消虚拟空间的地址映射 */
    while (space != NULL) {
        if (space->start >= VMS_MAP_START_ADDR &&
            space->end <= VMS_MAP_START_ADDR + MAX_VMS_MAP_SIZE) {
            page_unmap_addr_safe(space->start, space->end - space->start, space->flags & VMS_MAP_SHARED);
        }
        space = space->next;
    }
    return 0;
}

int vmm_exit(vmm_t *vmm)
{
    if (vmm == NULL)
        return -1; 
    if (vmm->vmspace_head == NULL)
        return -1;

    /* 取消虚拟空间映射 */
    if (vmm_unmap_space(vmm)) {
        return -1;
    }
    /* 释放虚拟空间描述 */
    if (vmm_release_space(vmm)) {
        return -1;
    }
    return 0;
}
