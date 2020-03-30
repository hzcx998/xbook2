#include <xbook/vmspace.h>
#include <xbook/task.h>
#include <xbook/debug.h>

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
 * GetUnmappedVMSpace - 获取一个没有映射的空闲空间
 * @mm: 内存管理器
 * @len: 要获取的长度
 * 
 * 查找一个没有映射的空间并返回其地址
 */
static unsigned long vmspace_get_unmaped(vmm_t *vmm, unsigned len)
{
    /* 地址指向没有映射的空间的最开始处 */
    unsigned long addr = VMM_UNMAPPED_BASE;

    // 根据地址获取一个space
    vmspace_t *space = vmspace_find(vmm, addr);
    
    // 循环查找长度符合的空间
    while (space != NULL) {
        /* 如果到达最后，就退出查找 */
        if (USER_VMM_SIZE - len < addr)
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
 * @addr: 地址
 * @len: 长度
 * @prot: 页保护
 * @flags: 空间的标志
 * 
 * 做地址映射，进程才可以读写空间
 */
int do_vmspace_map(vmm_t *vmm, unsigned long addr, unsigned long len,
    unsigned long prot, unsigned long flags)
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
    map_pages_safe(addr, len, prot);

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
    vmspace_t* space = vmspace_find_prev(vmm, addr, prev);
    /* 没找到空间就返回 */
    if (space == NULL) {      
        printk(KERN_ERR "do_vmspace_unmap: not found the space!\n");
        return -1;
    }
        
    /* 保证地址是在空间范围内
        在DoBrk中，我们要执行DoUmmap。如果是第一次执行，那么DoUmmap就会失败而退出，
        那么我们DoBrk就不会成功，因此，在这里，我们返回-2，而在DoBrk中判断返回-1才失败。
        在其它情况下，我们判断不是0就说明DoMap失败，这是一个特例
     */
     if (addr < space->start || addr + len > space->end) {
        //printk(KERN_ERR "do_vmspace_unmap: addr out of space!\n");
        //printk(KERN_ERR "do_vmspace_unmap: space start %x end %x\n", space->start, space->end);
        return -2;
    }
        
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

    /* 取消空间后需要取消虚拟地址映射 */
    unmap_pages_safe(addr, len);
    printk("unmap done!\n");
    return 0;
}

/**
 * vmspace_mmap - 内存映射
 * @addr: 地址
 * @len: 长度
 * @prot: 页保护
 * @flags: 空间的标志
 */
void *vmspace_mmap(uint32_t addr, uint32_t len, uint32_t prot, uint32_t flags)
{
    task_t *current = current_task;
    return (void *)do_vmspace_map(current->vmm, addr, len, prot, flags);
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