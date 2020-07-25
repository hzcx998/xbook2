#include <xbook/vmspace.h>
#include <xbook/task.h>
#include <xbook/debug.h>

#define DEBUG_LOCAL 0

void dump_vmspace(vmm_t *vmm)
{
    if (vmm == NULL)
        return;
    
    vmspace_t *space = vmm->vmspace_head;
    while (space != NULL) {
        printk(KERN_INFO "space: start=%x end=%x prot=%x flags:%x\n", 
            space->start, space->end, space->page_prot, space->flags);
        space = space->next;
    }
}

/* 虚拟空间的插入。*/
void vmspace_insert(vmm_t *vmm, vmspace_t *space)
{
    vmspace_t *prev = NULL;
    vmspace_t *p = (vmspace_t *)vmm->vmspace_head;
    while (p != NULL) {
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
        vmm->vmspace_head = (void *)space;
    space->vmm = vmm; /* 绑定空间的虚拟内存管理 */

    /* 合并相邻的空间 */
    /* merge prev and space */
    if (prev != NULL && prev->end == space->start) {
        if (prev->page_prot == space->page_prot && prev->flags == space->flags) {
            prev->end = space->end;
            prev->next = p;
            vmspace_free(space);
            space = prev;
        }
    }

    /* merge space and p */
    if (p != NULL && space->end == p->start) {
        if (space->page_prot == p->page_prot && space->flags == p->flags) {
            space->end = p->end;
            space->next = p->next;
            vmspace_free(space);    
        }
    }
}


/**
 * vmspace_get_unmaped - 获取一个没有映射的空闲空间
 * @mm: 内存管理器
 * @len: 要获取的长度
 * 
 * 查找一个没有映射的空间并返回其地址
 */
unsigned long vmspace_get_unmaped(vmm_t *vmm, unsigned len)
{
    /* 地址指向没有映射的空间的最开始处 */
    unsigned long addr = vmm->map_start;

    // 根据地址获取一个space
    vmspace_t *space = vmspace_find(vmm, addr);
    
    // 循环查找长度符合的空间
    while (space != NULL) {
        /* 如果到达最后，就退出查找 */
        if (USER_VMM_SIZE - len < addr)
            return -1;

        /* 如果超过可映射胡最大范围，也退出 */
        if (addr + len >= vmm->map_end)
            return -1;
    
        /* 如果要查找的区域在链表中间就直接返回地址 */
        if (addr + len <= space->start)
            return addr;
        
        /* 获取下一个地址 */
        addr = space->end;
        /* 获取下一个空间 */
        space = space->next;
    }
    /* 地址空间在链表的最后面 */
    return addr;
}

/** 
 * do_vmspace_map - 映射地址
 * @addr: 虚拟地址
 * @paddr: 虚拟地址
 * @len: 长度
 * @prot: 页保护
 * @flags: 空间的标志
 * 
 * 做地址映射，进程才可以读写空间
 */
int do_vmspace_map(vmm_t *vmm, unsigned long addr, unsigned long paddr, 
    unsigned long len, unsigned long prot, unsigned long flags)
{
    if (vmm == NULL || !prot) {
        printk(KERN_ERR "do_vmspace_map: failed!\n");
        return -1;
    }
    
    // printk(KERN_DEBUG "do_vmspace_map: %x, %x, %x, %x\n", addr, len, prot, flags);
    /* 让长度和页大小PAGE_SIZE对齐  */
    len = PAGE_ALIGN(len);

    /* 如果长度为0，什么也不做 */
    if (!len) {
        printk(KERN_ERR "do_vmspace_map: len is zero!\n");
        return -1;
    }

    /* 越界就返回 */
    if (len > USER_VMM_SIZE || addr > USER_VMM_SIZE || addr > USER_VMM_SIZE - len) {
        printk(KERN_ERR "do_vmspace_map: addr and len out of range!\n");
        return -1;
    }
    //printk(KERN_DEBUG "len right\n");
    
    /* 如果是MAP_FIXED, 地址就要和页大小PAGE_SIZE对齐 
    也就是说，传入的地址是多少就是多少，不用去自动协调
    */
    if (flags & VMS_MAP_FIXED) {
        // 地址需要是页对齐的
        if (addr & ~PAGE_MASK)
            return -1;

        /* 检测地址不在空间里面，也就是说 [addr, addr+len] */
        vmspace_t* p = vmspace_find(vmm, addr);

        // 存在这个空间，并且地址在这个空间中就返回。不能对这段空间映射
        if (p != NULL && addr + len > p->start) {
            printk(KERN_ERR "do_vmspace_map: this FIXED space had existed!\n");
            return -1;
        }
    } else {
        // 获取一个没有映射的空间，等会儿用来进行映射
        addr = vmspace_get_unmaped(vmm, len);
        if (addr == -1) {
            printk(KERN_ERR "do_vmspace_map: GetUnmappedVMSpace failed!\n");
            return -1;
        }
    }
    
    if (flags & VMS_MAP_REMAP) {
        prot |= PROT_REMAP;
    }

    /* 从slab中分配一块内存来当做VMSpace结构 */
    vmspace_t *space = vmspace_alloc();
    if (!space) {
        printk(KERN_ERR "do_vmspace_map: kmalloc for space failed!\n");
        return -1;    
    }

    vmspace_init(space, addr, addr + len, prot, flags);

    /* 插入空间到链表中，并且尝试合并 */
    vmspace_insert(vmm, space);

    //printk(KERN_DEBUG "map virtual from %x to %x\n", space->start, space->end);
    /* 创建空间后，需要做虚拟地址映射 */
    if (flags & VMS_MAP_SHARED) { /* 如果是共享映射，就映射成共享的地址 */
        //printk(KERN_DEBUG "do_vmspace_map: shared at %x:%x %x\n", addr, paddr, len);
        map_pages_fixed(addr, paddr, len, prot);
    } else {
        map_pages_safe(addr, len, prot); 
    }
    
    return addr;
}


/** 
 * do_vmspace_unmap - 取消一个空间的映射
 * @mm: 内存管理器
 * @addr: 空间的地址
 * @len: 空间的长度
 * 
 * 取消空间的映射，用于进程退出时使用，
 * 成功返回0，失败返回-1和-2。检测空间范围失败返回-2
 */
int do_vmspace_unmap(vmm_t *vmm, unsigned long addr, unsigned long len)
{
    /* 地址要和页大小PAGE_SIZE对齐 */
    if ((addr & ~PAGE_MASK) || addr > USER_VMM_SIZE || len > USER_VMM_SIZE-addr) {
        printk(KERN_ERR "do_vmspace_unmap: addr and len error!\n");
        return -1;
    }

    /* 让长度和页大小PAGE_SIZE对齐  */
    len = PAGE_ALIGN(len);

    /* 如果长度为0，什么也不做 */
    if (!len) {
        printk(KERN_ERR "do_vmspace_unmap: len is zero!\n");
        return -1;
    }
        
    /* 找到addr < space->end 的空间 */
    vmspace_t* prev = NULL;
    vmspace_t* space = vmspace_find_prev(vmm, addr, &prev);
    /* 没找到空间就返回 */
    if (space == NULL) {      
        printk(KERN_ERR "do_vmspace_unmap: not found the space!\n");
        return -1;
    }
    
    /* 保证地址是在空间范围内
        在do_vmspace_heap中，我们要执行do_vmspace_unmap。如果是第一次执行，那么do_vmspace_unmap就会失败而退出，
        那么我们do_vmspace_heap就不会成功，因此，在这里，我们返回-2，而在do_vmspace_heap中判断返回-1才失败。
        在其它情况下，我们判断不是0就说明do_vmspace_map失败，这是一个特例
     */
    if (addr < space->start || addr + len > space->end) {
        /*printk(KERN_NOTICE "do_vmspace_unmap: addr out of space!\n");
        printk(KERN_NOTICE "do_vmspace_unmap: space start %x end %x\n", addr, addr + len);
        printk(KERN_NOTICE "do_vmspace_unmap: space start %x end %x\n", space->start, space->end);*/

        return -2;
    }
    
    unmap_pages_safe(addr, len, space->flags & VMS_MAP_SHARED);

    /* 分配一个新的空间，有可能要unmap的空间会分成2个空间，例如：
    [start, addr, addr+len, end] => [start, addr], [addr+len, end]
     */

    vmspace_t* space_new = vmspace_alloc();
    if (!space_new) {        
        printk(KERN_ERR "do_vmspace_unmap: kmalloc for space_new failed!\n");
        return -1;
    }

    /* 把新空间链接到链表，新空间位于旧空间后面 */
    space_new->start = addr + len;
    space_new->end = space->end;
    space->end = addr;
    space_new->next = space->next;
    space->next = space_new;

    /* 检查是否是第一部分需要移除 */
    if (space->start == space->end) {
        vmspace_remove(vmm, space, prev);
        space = prev;
    }

    /* 检查是否是第二部分需要移除 */
    if (space_new->start == space_new->end) {
        vmspace_remove(vmm, space_new, space);
    }
    return 0;
}

/**
 * vmspace_mmap - 内存映射
 * @addr: 虚拟地址
 * @paddr: 物理地址
 * @len: 长度
 * @prot: 页保护
 * @flags: 空间的标志
 */
void *vmspace_mmap(uint32_t addr, uint32_t paddr, uint32_t len, uint32_t prot, uint32_t flags)
{
    task_t *current = current_task;
    return (void *)do_vmspace_map(current->vmm, addr, paddr, len, prot, flags);
}

/**
 * vmspace_unmmap - 取消内存映射
 * @addr: 地址
 * @len: 长度
 */
int vmspace_unmmap(uint32_t addr, uint32_t len)
{
    task_t *current = current_task;
    return do_vmspace_unmap(current->vmm, addr, len);
}



/**
 * do_vmspace_heap - 添加新的堆空间
 * @addr: 地址
 * @len: 长度 
 * 
 * 把地址和长度范围内的空间纳入堆的管理。
 */
static unsigned long do_vmspace_heap(vmm_t *vmm, unsigned long addr, unsigned long len)
{
    /* 页对齐后检查长度，如果为0就返回 */
    len = PAGE_ALIGN(len);
    if (!len) 
        return addr;
    
    //printk(KERN_DEBUG "do_vmspace_heap: addr %x len %x\n", addr, len);
    vmspace_t *space;
    unsigned long flags, ret;
    
    /* 先清除旧的空间，再进行新的映射 */
    ret = do_vmspace_unmap(vmm, addr, len);
    /* 如果返回值是-1，就说明取消映射失败 */
    if (ret == -1)
        return ret;

    /* 检测映射空间的数量是否超过最大数量 */

    flags = VMS_MAP_HEAP;

    /* 查看是否可以和原来的空间进行合并 */
    if (addr) {
        /* 查找一个比自己小的临近空间 */
        space = vmspace_find(vmm, addr - 1);
        /* 如果空间的结束和当前地址一样，并且flags也是一样的，就说明他们可以合并 */
        if (space && space->end == addr && space->flags == flags) {
            /*printk(KERN_DEBUG "do_vmspace_heap: space can merge. old space [%x-%x], new space [%x-%x]\n", 
                space->start, space->end, addr, addr + len);*/
            space->end = addr + len;
            goto the_end;
        }
    }

    /* 创建一个space，用来保存新的地址空间 */
    space = vmspace_alloc();
    if (space == NULL)
        return -1;
    vmspace_init(space, addr, addr + len, PROT_USER | PROT_WRITE | PROT_EXEC, flags);
    
    vmspace_insert(vmm, space);
    /*printk(KERN_DEBUG "do_vmspace_heap: insert space sucess! space [%x-%x]\n", 
        space->start, space->end);*/
the_end:
    return addr;
}

/**
 * sys_vmspace_heap - 设置堆的结束值
 * @heap: 堆值
 * 
 * 返回扩展的前一个地址
 * 如果heap为0，就返回当前heap
 * 如果大于vmm->heap_end，就向后扩展
 * 小于就向前缩小
 * 总是返回当前heap最新值
 */
unsigned long sys_vmspace_heap(unsigned long heap)
{
    unsigned long ret;
    unsigned long old_heap, new_heap;
    vmm_t *vmm = current_task->vmm;
#if DEBUG_LOCAL == 1    
    printk(KERN_DEBUG "%s: task %s pid %d vmm heap start %x end %x new %x\n", 
        __func__, current_task->name, current_task->pid, vmm->heap_start, vmm->heap_end, heap);
#endif
    /* 如果堆比开始位置都小就退出 */
    if (heap < vmm->heap_start) {
        //printk(KERN_DEBUG "sys_vmspace_heap: new heap too low!\n");
        goto the_end;
    }
    
    /* 使断点值和页对齐 */
    new_heap = PAGE_ALIGN(heap);
    old_heap = PAGE_ALIGN(vmm->heap_end);

    /* 如果新旧堆值在同一个页内，就设置新的堆值然后返回 */
    if (new_heap == old_heap) {
        //printk(KERN_DEBUG "sys_vmspace_heap: both in a page!\n");
        goto set_heap; 
    }
    
    /* 如果heap小于当前vmm的heap_end，就说明是收缩内存 */
    if (heap <= vmm->heap_end && heap >= vmm->heap_start) {
        //printk(KERN_DEBUG "sys_vmspace_heap: shrink mm.\n");
        
        /* 收缩地址就取消映射，如果成功就去设置新的断点值 */
        if (!do_vmspace_unmap(vmm, new_heap, old_heap - new_heap))
            goto set_heap;
        printk(KERN_ERR "sys_vmspace_heap: do_vmspace_unmap failed!\n");
        goto the_end;
    }
    
    /* 检查是否超过堆的空间限制 */
    if (heap > vmm->heap_start + MAX_VMS_HEAP_SIZE) {
        printk(KERN_ERR "%s: %x out of heap boundary!\n", __func__, heap);
        goto the_end;
    }
        
    vmspace_t *find;
    /* 检查是否和已经存在的空间发生重叠 */
    if ((find = vmspace_find_intersection(vmm, old_heap, new_heap + PAGE_SIZE))) {
        printk(KERN_ERR "%s: space intersection! old=%x, new=%x, end=%x\n",
            __func__, old_heap, new_heap, new_heap + PAGE_SIZE);
#if DEBUG_LOCAL == 1   
        printk(KERN_ERR "%s: find: start=%x, end=%x\n",
            __func__, find->start, find->end);

        printk(KERN_ERR "task=%d.\n", current_task->pid);
#endif
        goto the_end;
    }
    
    /* 检查是否有足够的内存可以进行扩展堆 */

    /* 堆新的断点进行空间映射 */
    if (do_vmspace_heap(vmm, old_heap, new_heap - old_heap) != old_heap) {
        printk(KERN_ERR "sys_vmspace_heap: do_heap failed! addr %x len %x\n",
            old_heap, new_heap - old_heap);
        goto the_end;
    }
     
set_heap:
#if DEBUG_LOCAL == 1   
    printk(KERN_DEBUG "sys_vmspace_heap: set new heap %x old is %x\n",
        heap, vmm->heap_end);
#endif
    vmm->heap_end = heap; /* 记录新的堆值 */
        
the_end:
    /* 获取mm中的heap值 */    
    ret = vmm->heap_end;
    //printk(KERN_DEBUG "ret heap is %x\n", ret);
    return ret;
}


