#include <arch/phymem.h>
#include <arch/vmm.h>
#include <xbook/vmm.h>
#include <xbook/debug.h>
#include <xbook/memspace.h>
#include <xbook/sharemem.h>
#include <string.h>

void vmm_init(vmm_t *vmm)
{
    vmm->page_storage = kern_page_copy_storge();
    if (vmm->page_storage == NULL) {
        panic(KERN_EMERG "task_init_vmm: mem_alloc for page_storege failed!\n");
    }
    vmm->mem_space_head = NULL;
}

void vmm_free(vmm_t *vmm)
{
    if (vmm) {
        if (vmm->page_storage) {
            page_free(kern_vir_addr2phy_addr(vmm->page_storage));
            vmm->page_storage = NULL;
        }
        mem_free(vmm);
    }
}

int sys_mstate(mstate_t *ms)
{
    if (!ms)
        return -1;
    memset(ms, 0, sizeof(mstate_t));
    ms->ms_total = mem_get_total_page_nr() * PAGE_SIZE;
    ms->ms_free = mem_get_free_page_nr() * PAGE_SIZE;
    ms->ms_used = ms->ms_total - ms->ms_free;
    if (ms->ms_used < 0)
        ms->ms_used = 0;
    return 0;
}

void vmm_dump(vmm_t *vmm)
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
            printk(KERN_ERR "copy_vm_mem_space: mem_alloc for space failed!\n");
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
                printk(KERN_ERR "vmm: release space on share map space [%x-%x]\n", space->start, space->end);
        }
        space = space->next;
        mem_space_free(p);
    }
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

int vmm_unmap_space(vmm_t *vmm)
{
    if (vmm == NULL)
        return -1;
    mem_space_t *space = (mem_space_t *)vmm->mem_space_head;
    while (space != NULL) {
        page_unmap_addr_safe(space->start, space->end - space->start, space->flags & MEM_SPACE_MAP_SHARED);
        space = space->next;
    }
    return 0;
}

/* 只取消空间中的映射部分的映射 */
int vmm_unmap_the_mapping_space(vmm_t *vmm)
{
    if (vmm == NULL)
        return -1; 
    mem_space_t *space = (mem_space_t *)vmm->mem_space_head;
    while (space != NULL) {
        if (space->start >= MEM_SPACE_MAP_ADDR_START &&
            space->end <= MEM_SPACE_MAP_ADDR_START + MAX_MEM_SPACE_MAP_SIZE) {
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
        printk(KERN_WARING "vmm: exit when unmap space failed!\n");
    }
    if (vmm_release_space(vmm)) {
        printk(KERN_WARING "vmm: exit when release space failed!\n");
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
        printk(KERN_WARING "vmm: exit when unmap space failed!\n");
    }
    vmm_active(parent_vmm); // active back to parent vmm 
    if (vmm_release_space(child_vmm)) {
        printk(KERN_WARING "vmm: exit when release space failed!\n");
    }
    vmm_free(child_vmm);    // free vmm, not used after this func.
    return 0;
}
