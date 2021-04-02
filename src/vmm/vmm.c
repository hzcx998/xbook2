#include <arch/phymem.h>
#include <arch/vmm.h>
#include <xbook/vmm.h>
#include <xbook/debug.h>
#include <xbook/memspace.h>
#include <xbook/sharemem.h>
#include <xbook/safety.h>
#include <xbook/process.h>
#include <string.h>
#include <errno.h>

void vmm_init(vmm_t *vmm)
{
    vmm->page_storage = kern_page_copy_storge();
    if (vmm->page_storage == NULL) {
        panic(PRINT_EMERG "task_init_vmm: mem_alloc for page_storege failed!\n");
    }
    vmm->mem_space_head = NULL;
    vmm->argv = NULL;
    vmm->envp = NULL;
    vmm->argbuf = NULL;
}

void vmm_free(vmm_t *vmm)
{
    if (vmm) {
        if (vmm->page_storage) {
            page_free(kern_vir_addr2phy_addr(vmm->page_storage));
            vmm->page_storage = NULL;
        }
        vmm_debuild_argbuf(vmm);
        mem_free(vmm);
    }
}

int sys_mstate(mstate_t *ms)
{
    if (!ms)
        return -EINVAL;
    mstate_t tms;
    tms.ms_total = mem_get_total_page_nr() * PAGE_SIZE;
    tms.ms_free = mem_get_free_page_nr() * PAGE_SIZE;
    tms.ms_used = tms.ms_total - tms.ms_free;
    if (tms.ms_used < 0)
        tms.ms_used = 0;
    if (mem_copy_to_user(ms, &tms, sizeof(mstate_t)) < 0) {
        return -EFAULT;
    }
    return 0;
}

void vmm_dump(vmm_t *vmm)
{
    keprint(PRINT_DEBUG "code: start=%x, end=%x\n", vmm->code_start, vmm->code_end);
    keprint(PRINT_DEBUG "data: start=%x, end=%x\n", vmm->data_start, vmm->data_end);
    keprint(PRINT_DEBUG "heap: start=%x, end=%x\n", vmm->heap_start, vmm->heap_end);
    keprint(PRINT_DEBUG "map: start=%x, end=%x\n", vmm->map_start, vmm->map_end);
    keprint(PRINT_DEBUG "stack: start=%x, end=%x\n", vmm->stack_start, vmm->stack_end);
}

void vmm_active(vmm_t *vmm)
{
    if (vmm == NULL) {
        vmm_active_kernel();
    } else {   
        vmm_active_user(kern_vir_addr2phy_addr(vmm->page_storage));
    }
}

int vmm_dec_share_mem(mem_space_t *mem_space)
{
    addr_t phyaddr = addr_vir2phy(mem_space->start);  
    share_mem_t *shm = share_mem_find_by_addr(phyaddr);
    if (shm == NULL) { 
        return 0;
    }
    return share_mem_dec(shm->id);
}

int vmm_inc_share_mem(mem_space_t *mem_space)
{
    addr_t phyaddr = addr_vir2phy(mem_space->start);  
    share_mem_t *shm = share_mem_find_by_addr(phyaddr);
    if (shm == NULL) { 
        return 0;
    }
    return share_mem_inc(shm->id);
}

int vmm_copy_mem_space(vmm_t *child_vmm, vmm_t *parent_vmm)
{
    mem_space_t *tail = NULL;
    mem_space_t *p = parent_vmm->mem_space_head;
    while (p != NULL) {
        mem_space_t *space = mem_space_alloc();
        if (space == NULL) {
            keprint(PRINT_ERR "copy_vm_mem_space: mem_alloc for space failed!\n");
            return -1;
        }
        *space = *p;
        space->next = NULL;
        if (space->flags & MEM_SPACE_MAP_SHARED) {
            if (vmm_inc_share_mem(space) < 0)
                return -1;
        }
        if (tail == NULL)
            child_vmm->mem_space_head = space;    
        else 
            tail->next = space;
        tail = space;
        p = p->next;
    }
    return 0;
}

int vmm_release_space(vmm_t *vmm)
{
    if (vmm == NULL)
        return -1; 
    mem_space_t *space = (mem_space_t *)vmm->mem_space_head;
    mem_space_t *p;
    while (space != NULL) {
        p = space;
        if (space->flags & MEM_SPACE_MAP_SHARED) {
            if (vmm_dec_share_mem(space) < 0)
                keprint(PRINT_ERR "vmm: release space on share map space [%x-%x]\n", space->start, space->end);
        }
        space = space->next;
        mem_space_free(p);
    }
    vmm_debuild_argbuf(vmm);
    vmm->mem_space_head = NULL;
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
 * BUG: 当执行内存取消映射时，就会产生内存bug。
 */
int vmm_unmap_space(vmm_t *vmm)
{
    if (vmm == NULL)
        return -1;
    mem_space_t *space = (mem_space_t *)vmm->mem_space_head;
    while (space != NULL) {
        /* 堆栈和代码数据的映射和解除映射有所不同，需要单独处理 */
        if ((space->flags & MEM_SPACE_MAP_STACK) || (space->flags & MEM_SPACE_MAP_HEAP)) {
            page_unmap_addr(space->start, space->end - space->start);
        } else {
            page_unmap_addr_safe(space->start, space->end - space->start, space->flags & MEM_SPACE_MAP_SHARED);
        }
        space = space->next;
    }
    return 0;
}

/* 取消动态映射部分，进程执行前都需要确保这片区域是没有被映射的 */
int vmm_unmap_the_mapping_space(vmm_t *vmm)
{
    if (vmm == NULL)
        return -1; 
    mem_space_t *space = (mem_space_t *)vmm->mem_space_head;
    while (space != NULL) {
        if (space->start >= vmm->map_start &&
            space->end <= vmm->map_end) {
            page_unmap_addr_safe(space->start, space->end - space->start, space->flags & MEM_SPACE_MAP_SHARED);
        }
        space = space->next;
    }
    return 0;
}

int vmm_exit(vmm_t *vmm)
{
    if (vmm == NULL)
        return -1; 
    
    if (vmm->mem_space_head == NULL) {
        return -1;
    }
    
    if (vmm_unmap_space(vmm)) {
        keprint(PRINT_WARING "vmm: exit when unmap space failed!\n");
    }
    
    if (vmm_release_space(vmm)) {
        keprint(PRINT_WARING "vmm: exit when release space failed!\n");
    }
    
    return 0;
}

int vmm_exit_when_fork_failed(vmm_t *child_vmm, vmm_t *parent_vmm)
{
    if (child_vmm == NULL)
        return -1; 
    
    if (child_vmm->mem_space_head == NULL) {
        return -1;
    }
    vmm_active(child_vmm); // active child vmm for unmap space
    if (vmm_unmap_space(child_vmm)) {
        keprint(PRINT_WARING "vmm: exit when unmap space failed!\n");
    }
    vmm_active(parent_vmm); // active back to parent vmm 
    if (vmm_release_space(child_vmm)) {
        keprint(PRINT_WARING "vmm: exit when release space failed!\n");
    }
    vmm_free(child_vmm);    // free vmm, not used after this func.
    return 0;
}

int vmm_build_argbug(vmm_t *vmm, char **argv, char **envp)
{
    char *tmp_arg = mem_alloc(PAGE_SIZE);
    if (!tmp_arg) {
        return -1;
    }
    memset(tmp_arg, 0, PAGE_SIZE); 
    /* 构建参数缓冲区 */
    unsigned long arg_bottom;
    proc_build_arg((unsigned long) tmp_arg + PAGE_SIZE, &arg_bottom, (char **) envp, &vmm->envp);
    proc_build_arg(arg_bottom, NULL, (char **) argv, &vmm->argv);
    vmm->argbuf = tmp_arg;
    return 0;
}

void vmm_debuild_argbuf(vmm_t *vmm)
{
    if (vmm->argbuf) {
        mem_free(vmm->argbuf);
        vmm->argbuf = NULL;
        vmm->argv = NULL;
        vmm->envp = NULL;
    }
}
