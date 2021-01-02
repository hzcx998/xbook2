#include <xbook/memspace.h>
#include <xbook/task.h>
#include <xbook/debug.h>
#include <xbook/schedule.h>

// #define DEBUG_MEM_SPACE

void mem_space_dump(vmm_t *vmm)
{
    if (vmm == NULL)
        return;
    mem_space_t *space = vmm->mem_space_head;
    while (space != NULL) {
        keprint(PRINT_INFO "space: start=%x end=%x prot=%x flags:%x\n", 
            space->start, space->end, space->page_prot, space->flags);
        space = space->next;
    }
}

void mem_space_insert(vmm_t *vmm, mem_space_t *space)
{
    mem_space_t *prev = NULL;
    mem_space_t *p = (mem_space_t *)vmm->mem_space_head;
    while (p != NULL) {
        if (space->end <= p->start)
            break;
        prev = p;
        p = p->next;
    }
    space->next = p;
    if (prev)
        prev->next = space;
    else
        vmm->mem_space_head = (void *)space;
    space->vmm = vmm;
    /* 共享内存不进行合并处理 */
    if (space->flags & MEM_SPACE_MAP_SHARED) {
        return;
    }
    /* merge prev and space */
    if (prev != NULL && prev->end == space->start) {
        if (prev->page_prot == space->page_prot && prev->flags == space->flags) {
            prev->end = space->end;
            prev->next = p;
            mem_space_free(space);
            space = prev;
        }
    }
    /* merge space and p */
    if (p != NULL && space->end == p->start) {
        if (space->page_prot == p->page_prot && space->flags == p->flags) {
            space->end = p->end;
            space->next = p->next;
            mem_space_free(space);    
        }
    }
}

unsigned long mem_space_get_unmaped(vmm_t *vmm, unsigned len)
{
    unsigned long addr = vmm->map_start;
    mem_space_t *space = mem_space_find(vmm, addr);
    while (space != NULL) {
        if (USER_VMM_SIZE - len < addr)
            return -1;
        if (addr + len >= vmm->map_end)
            return -1;
        if (addr + len <= space->start)
            return addr;
    
        addr = space->end;
        space = space->next;
    }
    return addr;
}

int do_mem_space_map(vmm_t *vmm, unsigned long addr, unsigned long paddr, 
    unsigned long len, unsigned long prot, unsigned long flags)
{
    if (vmm == NULL || !prot) {
        keprint(PRINT_ERR "do_mem_space_map: failed!\n");
        return -1;
    }
    len = PAGE_ALIGN(len);
    if (!len) {
        keprint(PRINT_ERR "do_mem_space_map: len is zero!\n");
        return -1;
    }
    if (len > USER_VMM_SIZE || addr > USER_VMM_SIZE || addr > USER_VMM_SIZE - len) {
        keprint(PRINT_ERR "do_mem_space_map: addr and len out of range!\n");
        return -1;
    }
    if (flags & MEM_SPACE_MAP_FIXED) {
        if (addr & ~PAGE_MASK)
            return -1;
        mem_space_t* p = mem_space_find(vmm, addr);
        if (p != NULL && addr + len > p->start) {
            keprint(PRINT_ERR "do_mem_space_map: this FIXED space had existed!\n");
            return -1;
        }
    } else {
        addr = mem_space_get_unmaped(vmm, len);
        if (addr == -1) {
            keprint(PRINT_ERR "do_mem_space_map: GetUnmappedMEM_SPACEpace failed!\n");
            return -1;
        }
    }
    if (flags & MEM_SPACE_MAP_REMAP) {
        prot |= PROT_REMAP;
    }
    mem_space_t *space = mem_space_alloc();
    if (!space) {
        keprint(PRINT_ERR "do_mem_space_map: mem_alloc for space failed!\n");
        return -1;    
    }
    mem_space_init(space, addr, addr + len, prot, flags);
    mem_space_insert(vmm, space);
    /* 如果是共享映射，就映射成共享的地址，需要指定物理地址 */
    if (flags & MEM_SPACE_MAP_SHARED) {
        page_map_addr_fixed(addr, paddr, len, prot);
    } else {
        page_map_addr_safe(addr, len, prot); 
    }
    return addr;
}

int do_mem_space_unmap(vmm_t *vmm, unsigned long addr, unsigned long len)
{
    if ((addr & ~PAGE_MASK) || addr > USER_VMM_SIZE || len > USER_VMM_SIZE-addr) {
        keprint(PRINT_ERR "do_mem_space_unmap: addr and len error!\n");
        return -1;
    }
    len = PAGE_ALIGN(len);
    if (!len) {
        keprint(PRINT_ERR "do_mem_space_unmap: len is zero!\n");
        return -1;
    }
    mem_space_t* prev = NULL;
    mem_space_t* space = mem_space_find_prev(vmm, addr, &prev);
    if (space == NULL) {      
        keprint(PRINT_ERR "do_mem_space_unmap: not found the space!\n");
        return -1;
    }
    
    if (addr < space->start || addr + len > space->end) {
        return 0;
    }
    
    page_unmap_addr_safe(addr, len, space->flags & MEM_SPACE_MAP_SHARED);
    mem_space_t* space_new = mem_space_alloc();
    if (!space_new) {        
        keprint(PRINT_ERR "do_mem_space_unmap: mem_alloc for space_new failed!\n");
        return -1;
    }
    space_new->start = addr + len;
    space_new->end = space->end;
    space->end = addr;
    space_new->next = space->next;
    space->next = space_new;
    if (space->start == space->end) {
        mem_space_remove(vmm, space, prev);
        space = prev;
    }
    if (space_new->start == space_new->end) {
        mem_space_remove(vmm, space_new, space);
    }
    return 0;
}

void *mem_space_mmap(uint32_t addr, uint32_t paddr, uint32_t len, uint32_t prot, uint32_t flags)
{
    task_t *current = task_current;
    return (void *)do_mem_space_map(current->vmm, addr, paddr, len, prot, flags);
}

int mem_space_unmmap(uint32_t addr, uint32_t len)
{
    task_t *current = task_current;
    return do_mem_space_unmap(current->vmm, addr, len);
}

static unsigned long do_mem_space_expend_heap(vmm_t *vmm, unsigned long addr, unsigned long len)
{
    len = PAGE_ALIGN(len);
    if (!len) 
        return addr;
    mem_space_t *space;
    unsigned long flags, ret;
    ret = do_mem_space_unmap(vmm, addr, len);
    if (ret == -1)
        return ret;
    flags = MEM_SPACE_MAP_HEAP;
    if (addr) {
        space = mem_space_find(vmm, addr - 1);
        if (space && space->end == addr && space->flags == flags) {
            space->end = addr + len;
            goto the_end;
        }
    }
    space = mem_space_alloc();
    if (space == NULL)
        return -1;
    mem_space_init(space, addr, addr + len, PROT_USER | PROT_WRITE | PROT_EXEC, flags);
    mem_space_insert(vmm, space);
the_end:
    return addr;
}

unsigned long sys_mem_space_expend_heap(unsigned long heap)
{
    unsigned long ret;
    unsigned long old_heap, new_heap;
    vmm_t *vmm = task_current->vmm;
#ifdef DEBUG_MEM_SPACE    
    keprint(PRINT_DEBUG "%s: task %s pid %d vmm heap start %x end %x new %x\n", 
        __func__, task_current->name, task_current->pid, vmm->heap_start, vmm->heap_end, heap);
#endif
    /* 如果堆比开始位置都小就退出 */
    if (heap < vmm->heap_start) {
        goto the_end;
    }
    new_heap = PAGE_ALIGN(heap);
    old_heap = PAGE_ALIGN(vmm->heap_end);
    if (new_heap == old_heap) {
        goto set_heap; 
    }
    
    if (heap <= vmm->heap_end && heap >= vmm->heap_start) {
        if (!do_mem_space_unmap(vmm, new_heap, old_heap - new_heap))
            goto set_heap;
        keprint(PRINT_ERR "sys_mem_space_expend_heap: do_mem_space_unmap failed!\n");
        goto the_end;
    }
    
    if (heap > vmm->heap_start + MAX_MEM_SPACE_HEAP_SIZE) {
        keprint(PRINT_ERR "%s: %x out of heap boundary!\n", __func__, heap);
        goto the_end;
    }
    mem_space_t *find;
    if ((find = mem_space_find_intersection(vmm, old_heap, new_heap + PAGE_SIZE))) {
        keprint(PRINT_ERR "%s: space intersection! old=%x, new=%x, end=%x\n",
            __func__, old_heap, new_heap, new_heap + PAGE_SIZE);
#ifdef DEBUG_MEM_SPACE   
        keprint(PRINT_ERR "%s: find: start=%x, end=%x\n",
            __func__, find->start, find->end);

        keprint(PRINT_ERR "task=%d.\n", task_current->pid);
#endif
        goto the_end;
    }
    if (do_mem_space_expend_heap(vmm, old_heap, new_heap - old_heap) != old_heap) {
        keprint(PRINT_ERR "sys_mem_space_expend_heap: do_heap failed! addr %x len %x\n",
            old_heap, new_heap - old_heap);
        goto the_end;
    }
set_heap:
#ifdef DEBUG_MEM_SPACE   
    keprint(PRINT_DEBUG "sys_mem_space_expend_heap: set new heap %x old is %x\n",
        heap, vmm->heap_end);
#endif
    vmm->heap_end = heap;
the_end:
    ret = vmm->heap_end;
    return ret;
}
