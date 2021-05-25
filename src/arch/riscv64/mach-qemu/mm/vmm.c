#include <arch/vmm.h>
#include <arch/page.h>
#include <arch/riscv.h>
#include <xbook/debug.h>
#include <xbook/task.h>
#include <xbook/memspace.h>

extern pgdir_t kernel_pgdir;

// #define DEBUG_ARCH_VMM

static int do_copy_share_page(addr_t vaddr, vmm_t *child, vmm_t *parent)
{
    uint64_t paddr;
    pte_t *pte;
    if((pte = walk((pgdir_t)parent->page_storage, vaddr, 0)) == NULL)
        panic("do_copy_normal_page: pte should exist");
    if((*pte & PAGE_ATTR_PRESENT) == 0)
        panic("do_copy_normal_page: page not present");

    /* get old page addr and perm */
    paddr = PTE2PA(*pte);
    int perm = PTE_FLAGS(*pte) & PAGE_ATTR_MASK;

    dbgprint("[vmm]: copy share page at vaddr %x phy addr %x\n", vaddr, paddr);   
    mappages(child->page_storage, vaddr, PAGE_SIZE, paddr, perm);
    return 0;
}

/**
 * 从父进程复制页到子进程
 * 1. 父进程复制到临时缓冲区
 * 2. 映射子进程的虚拟地址
 * 3. 将临时缓冲区数据复制到子进程
 */
static int do_copy_normal_page(addr_t vaddr, void *buf, vmm_t *child, vmm_t *parent)
{
    uint64_t paddr;
    uint64_t mem;
    pte_t *pte;

    if((pte = walk((pgdir_t)parent->page_storage, vaddr, 0)) == NULL)
        panic("do_copy_normal_page: pte should exist");
    if((*pte & PAGE_ATTR_PRESENT) == 0) {   /* 不存在，则不复制页，可能是没被访问的堆栈 */
        // warnprintln("[fork] do_copy_normal_page: page not present");
        return 0;
    }

    /* get old page addr and perm */
    paddr = PTE2PA(*pte);
    int perm = PTE_FLAGS(*pte) & PAGE_ATTR_MASK;
    // noteprint(PRINT_ERR "vmm_copy_mapping: vaddr %p perm %x!\n", vaddr, perm);
    
    mem = page_alloc_user(1);
    if (!mem) {
        errprint(PRINT_ERR "vmm_copy_mapping: page_alloc_one for vaddr failed!\n");
        return -1;
    }
    /* map page buf in child */
    if (mappages(child->page_storage, vaddr, PAGE_SIZE, mem, perm) < 0) {
        errprint(PRINT_ERR "vmm_copy_mapping: map page for vaddr %p failed!\n", vaddr);
        return -1;
    }

    /* copy data from kernel to user */
    memcpy((uint64_t *)mem, (uint64_t *)paddr, PAGE_SIZE);
    return 0;
}

int vmm_copy_mapping(task_t *child, task_t *parent)
{
    void *buf = mem_alloc(PAGE_SIZE);
    if (buf == NULL) {
        keprint(PRINT_ERR "vmm_copy_mapping: mem_alloc buf for data transform failed!\n");
        return -1;
    }
    mem_space_t *space = parent->vmm->mem_space_head;
    addr_t prog_vaddr = 0;
    while (space != NULL) {
        prog_vaddr = space->start;
        while (prog_vaddr < space->end) {
            /* 如果是共享内存，就只复制页映射，而不创建新的页 */
            if (space->flags & MEM_SPACE_MAP_SHARED) {
                if (do_copy_share_page(prog_vaddr, child->vmm, parent->vmm) < 0) {
                    mem_free(buf);
                    return -1;
                }
            } else {
                if (do_copy_normal_page(prog_vaddr, buf, child->vmm, parent->vmm) < 0) {
                    mem_free(buf);
                    return -1;
                }
            }
            prog_vaddr += PAGE_SIZE;
        }
        space = space->next;
    }
    mem_free(buf);
    return 0; 
}

void vmm_active_kernel()
{
    #ifdef DEBUG_ARCH_VMM
    keprintln("[vmm] vmm_active_kernel");
    #endif
    #if 0
    /* 如果时同一个页表就不改变，这样减少对页表的刷新，提高效率 */
    if (r_satp() == MAKE_SATP(kernel_pgdir))
        return;
    #endif
    /* 激活内核页表 */
    w_satp(MAKE_SATP(kernel_pgdir));
    sfence_vma();
}

void vmm_active_user(unsigned long page)
{
    #ifdef DEBUG_ARCH_VMM
    keprintln("[vmm] vmm_active_user");
    #endif
    #if 0
    if (r_satp() == MAKE_SATP(page))
        return;
    #endif
    /* 激活用户页表 */
    w_satp(MAKE_SATP(page));
    sfence_vma();
}
