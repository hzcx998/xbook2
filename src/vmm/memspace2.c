#include <xbook/memspace.h>
#include <xbook/task.h>
#include <xbook/debug.h>
#include <xbook/schedule.h>

// #define DEBUG_MEM_SPACE

void *do_mem_space_map2(vmm_t *vmm, unsigned long addr, unsigned long paddr, 
    unsigned long len, unsigned long prot, unsigned long flags)
{
    if (vmm == NULL || !prot) {
        keprint(PRINT_ERR "do_mem_space_map: failed!\n");
        return (void *)-1;
    }
    len = PAGE_ALIGN(len);
    if (!len) {
        keprint(PRINT_ERR "do_mem_space_map: len is zero!\n");
        return (void *)-1;
    }
    if (len > USER_VMM_SIZE || addr > USER_VMM_TOP_ADDR || addr > USER_VMM_TOP_ADDR - len || addr < USER_VMM_BASE_ADDR) {
        keprint(PRINT_ERR "do_mem_space_map: addr %x and len %x out of range!\n", addr, len);
        return (void *)-1;
    }
    if (flags & MEM_SPACE_MAP_FIXED) {
        if (addr & ~PAGE_MASK) {
            keprint(PRINT_ERR "do_mem_space_map: addr %x not page aligined!\n", addr);
            return (void *)-1;
        }
        mem_space_t* p = mem_space_find(vmm, addr);
        if (p != NULL && addr + len > p->start) {
            keprint(PRINT_ERR "do_mem_space_map: this FIXED space had existed!\n");
            return (void *)-1;
        }
    } else {
        addr = mem_space_get_unmaped(vmm, len);
        if (addr == -1) {
            keprint(PRINT_ERR "do_mem_space_map: GetUnmappedMEM_SPACEpace failed!\n");
            return (void *)-1;
        }
    }
    if (flags & MEM_SPACE_MAP_REMAP) {
        prot |= PROT_REMAP;
    }
    mem_space_t *space = mem_space_alloc();
    if (!space) {
        keprint(PRINT_ERR "do_mem_space_map: mem_alloc for space failed!\n");
        return (void *)-1;    
    }
    mem_space_init(space, addr, addr + len, prot, flags);
    mem_space_insert(vmm, space);
    /* 如果是共享映射，就映射成共享的地址，需要指定物理地址 */
    if (flags & MEM_SPACE_MAP_SHARED) {
        page_map_addr_fixed2(vmm->page_storage, addr, paddr, len, prot);
    } else {
        page_map_addr_safe2(vmm->page_storage, addr, len, prot); 
    }
    //keprint(PRINT_ERR "do_mem_space_map: addr %x.\n", addr);
    return (void *) addr;
}

void *do_mem_space_map_viraddr2(vmm_t *vmm, unsigned long addr, unsigned long vaddr, 
    unsigned long len, unsigned long prot, unsigned long flags)
{
    if (vmm == NULL || !prot) {
        keprint(PRINT_ERR "do_mem_space_map: failed!\n");
        return (void *)-1;
    }
    len = PAGE_ALIGN(len);
    if (!len) {
        keprint(PRINT_ERR "do_mem_space_map: len is zero!\n");
        return (void *)-1;
    }
    if (len > USER_VMM_SIZE || addr > USER_VMM_TOP_ADDR || addr > USER_VMM_TOP_ADDR - len || addr < USER_VMM_BASE_ADDR) {
        keprint(PRINT_ERR "do_mem_space_map_viraddr: addr %x and len %x out of range!\n", addr, len);
        return (void *)-1;
    }
    if (flags & MEM_SPACE_MAP_FIXED) {
        if (addr & ~PAGE_MASK)
            return (void *)-1;
        mem_space_t* p = mem_space_find(vmm, addr);
        if (p != NULL && addr + len > p->start) {
            keprint(PRINT_ERR "do_mem_space_map: this FIXED space had existed!\n");
            return (void *)-1;
        }
    } else {
        addr = mem_space_get_unmaped(vmm, len);
        if (addr == -1) {
            keprint(PRINT_ERR "do_mem_space_map: GetUnmappedMEM_SPACEpace failed!\n");
            return (void *)-1;
        }
    }
    if (flags & MEM_SPACE_MAP_REMAP) {
        prot |= PROT_REMAP;
    }
    mem_space_t *space = mem_space_alloc();
    if (!space) {
        keprint(PRINT_ERR "do_mem_space_map: mem_alloc for space failed!\n");
        return (void *)-1;    
    }
    mem_space_init(space, addr, addr + len, prot, flags);
    mem_space_insert(vmm, space);
    /* 如果是共享映射，就映射成共享的地址，需要指定物理地址 */
    if (flags & MEM_SPACE_MAP_SHARED) {
        /* 单页映射，避免虚拟地址不连续造成的映射问题 */
        unsigned long vend = vaddr + len;
        unsigned long paddr;
        unsigned long vstart = addr;
        while (vaddr < vend) {
            paddr = kern_vir_addr2phy_addr(vaddr);
            page_map_addr_fixed2(vmm->page_storage, vstart, paddr, PAGE_SIZE, prot);
            vaddr += PAGE_SIZE;
            vstart += PAGE_SIZE;
        }
    } else {
        page_map_addr_safe2(vmm->page_storage, addr, len, prot); 
    }
    return (void *)addr;
}

int do_mem_space_unmap2(vmm_t *vmm, unsigned long addr, unsigned long len)
{
    if ((addr & ~PAGE_MASK) || addr > USER_VMM_TOP_ADDR || addr > USER_VMM_TOP_ADDR - len || addr < USER_VMM_BASE_ADDR) {
        keprint(PRINT_ERR "do_mem_space_unmap: addr %x and len %x error!\n", addr, len);
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
        // 不在范围内，需要进行控件拓展
        // noteprint("unmap: addr out of range: addr%x -> [%x-%x]\n", addr, space->start, space->end);
        return 0;
    }
    page_unmap_addr_safe2(vmm->page_storage, addr, len, space->flags & MEM_SPACE_MAP_SHARED);

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

void *mem_space_mmap2(vmm_t *vmm, unsigned long addr, unsigned long paddr, unsigned long len, uint32_t prot, uint32_t flags)
{
    return (void *)do_mem_space_map2(vmm, addr, paddr, len, prot, flags);
}

void *mem_space_mmap_viraddr2(vmm_t *vmm, unsigned long addr, unsigned long vaddr, unsigned long len, uint32_t prot, uint32_t flags)
{
    return (void *)do_mem_space_map_viraddr2(vmm, addr, vaddr, len, prot, flags);
}

int mem_space_unmmap2(vmm_t *vmm, unsigned long addr, unsigned long len)
{
    return do_mem_space_unmap2(vmm, addr, len);
}
