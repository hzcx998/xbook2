#include <arch/page.h>
#include <arch/mempool.h>
#include <xbook/debug.h>
#include <arch/riscv.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <k210_phymem.h>
#include <xbook/task.h>
#include <xbook/schedule.h>

int page_map_addr_fixed2(pgdir_t pgdir, unsigned long start, unsigned long addr, 
    unsigned long len, unsigned long prot)
{
    unsigned long flags;
    interrupt_save_and_disable(flags);
    unsigned long first = start & PAGE_MASK;
    len = PAGE_ALIGN(len);

    unsigned long attr = 0;
    
    if (prot & PROT_USER)
        attr |= PAGE_ATTR_USER;
    
    if (prot & PROT_KERN)
        attr |= PAGE_ATTR_SYSTEM;
    
    if (prot & PROT_WRITE)
        attr |= PAGE_ATTR_WRITE;
    
    if (prot & PROT_READ)
        attr |= PAGE_ATTR_READ;

    if (prot & PROT_EXEC)
        attr |= PAGE_ATTR_EXEC;

    int retval = mappages(pgdir, first, len, addr & PAGE_MASK, attr);
    interrupt_restore_state(flags);
	return retval;
}

int page_unmap_addr2(pgdir_t pgdir, unsigned long vaddr, unsigned long len)
{
    if (!len)
		return -1;
    unsigned long flags;
    interrupt_save_and_disable(flags);
	len = PAGE_ALIGN(len);
    vmunmap2(pgdir, vaddr & PAGE_MASK, len / PAGE_SIZE, 1);
    interrupt_restore_state(flags);
	return 0;
}

int page_unmap_addr_safe2(pgdir_t pgdir, unsigned long start, unsigned long len, char fixed)
{
    if (!len)
		return -1;
    unsigned long flags;
    interrupt_save_and_disable(flags);
	len = PAGE_ALIGN(len);
    vmunmap2(pgdir, start & PAGE_MASK, len / PAGE_SIZE, !fixed);   /* 不是固定才释放 */
    interrupt_restore_state(flags);
	return 0;
}

int page_map_addr2(pgdir_t pgdir, unsigned long start, unsigned long len, unsigned long prot)
{
    //dbgprintln("[page] page_map_addr: start=%p len=%lx prot=%x\n", start, len, prot);
    unsigned long flags;
    interrupt_save_and_disable(flags);
    unsigned long vaddr = (unsigned long )start & PAGE_MASK;
    len = PAGE_ALIGN(len);
    unsigned long pages = DIV_ROUND_UP(len, PAGE_SIZE);
    unsigned long page_idx = 0;
    unsigned long page_addr = 0;
    unsigned long attr = 0;
    
    if (prot & PROT_USER)
        attr |= PAGE_ATTR_USER;
    
    if (prot & PROT_KERN)
        attr |= PAGE_ATTR_SYSTEM;
    
    if (prot & PROT_WRITE)
        attr |= PAGE_ATTR_WRITE;
    
    if (prot & PROT_READ)
        attr |= PAGE_ATTR_READ;

    if (prot & PROT_EXEC)
        attr |= PAGE_ATTR_EXEC;

    //dbgprintln("[page] page_map_addr: vaddr=%p pages=%d\n", vaddr, pages);
    
    int retval = -1;
    while (page_idx < pages) {
        page_addr = page_alloc_user(1);
        if (!page_addr) {
            keprint("error: user_map_vaddr -> map pages failed!\n");
            interrupt_restore_state(flags);
            return -1;
        }
        //dbgprintln("[page] page_map_addr: vaddr=%p paddr=%p\n", vaddr, page_addr);
        retval = mappages(pgdir, vaddr, PAGE_SIZE, page_addr, attr);
        if (retval < 0) {
            vmunmap(pgdir, start & PAGE_MASK, pages, 1);
            goto final;
        }
        vaddr += PAGE_SIZE;
        page_idx++;
    }
    interrupt_restore_state(flags);
final:
    return 0;
}

/**
 * 映射一片内存区域，如果虚拟地址对应的物理地址不存在才为其分配物理地址，已经存在就默认使用原来的物理页。
 */
int page_map_addr_safe2(pgdir_t pgdir, unsigned long start, unsigned long len, unsigned long prot)
{
    unsigned long flags;
    interrupt_save_and_disable(flags);
    unsigned long vaddr = (unsigned long )start & PAGE_MASK;
    len = PAGE_ALIGN(len);
    unsigned long pages = DIV_ROUND_UP(len, PAGE_SIZE);
    unsigned long page_idx = 0;
    unsigned long page_addr;
    unsigned long attr = 0;
    
    if (prot & PROT_USER)
        attr |= PAGE_ATTR_USER;
    
    if (prot & PROT_KERN)
        attr |= PAGE_ATTR_SYSTEM;
    
    if (prot & PROT_WRITE)
        attr |= PAGE_ATTR_WRITE;
    
    if (prot & PROT_READ)
        attr |= PAGE_ATTR_READ;

    if (prot & PROT_EXEC)
        attr |= PAGE_ATTR_EXEC;

    int retval = -1;
    pte_t *pte;
    while (page_idx < pages) {
        pte = walk(pgdir, vaddr, 0);
        if (pte && (*pte & PAGE_ATTR_PRESENT)) {    /* 地址已经存在了 */
            errprintln("page_map_addr_safe: addr %#x had maped!\n", vaddr);
        } else {
            page_addr = page_alloc_user(1);
            if (!page_addr) {
                keprint("error: user_map_vaddr -> map pages failed!\n");
                interrupt_restore_state(flags);
                return -1;
            }
            retval = mappages(pgdir, vaddr, PAGE_SIZE, page_addr, attr);
            if (retval < 0) {
                vmunmap(pgdir, start & PAGE_MASK, pages, 1);
                goto final;
            }
        }
        vaddr += PAGE_SIZE;
        page_idx++;
    }
    interrupt_restore_state(flags);
final:
    return 0;
}


// Copy from kernel to user.
// Copy len bytes from src to virtual address dstva in a given page table.
// Return 0 on success, -1 on error.
int
copyout(pgdir_t pgdir, uint64_t dstva, char *src, uint64_t len)
{
  uint64_t n, va0, *pa0;

  while(len > 0){
    va0 = PAGE_ROUNDDOWN(dstva);
    pa0 = walkaddr(pgdir, va0);
    if(pa0 == NULL)
      return -1;
    n = PAGE_SIZE - (dstva - va0);
    if(n > len)
      n = len;
    memmove((void *)((uint64_t)pa0 + (dstva - va0)), src, n);

    len -= n;
    src += n;
    dstva = va0 + PAGE_SIZE;
  }
  return 0;
}

// Copy from user to kernel.
// Copy len bytes to dst from virtual address srcva in a given page table.
// Return 0 on success, -1 on error.
int
copyin(pgdir_t pgdir, char *dst, uint64_t srcva, uint64_t len)
{
  uint64_t n, va0, *pa0;

  while(len > 0){
    va0 = PAGE_ROUNDDOWN(srcva);
    pa0 = walkaddr(pgdir, va0);
    if(pa0 == NULL)
      return -1;
    n = PAGE_SIZE - (srcva - va0);
    if(n > len)
      n = len;
    memmove(dst, (void *)((uint64_t)pa0 + (srcva - va0)), n);

    len -= n;
    dst += n;
    srcva = va0 + PAGE_SIZE;
  }
  return 0;
}


// Copy a null-terminated string from user to kernel.
// Copy bytes to dst from virtual address srcva in a given page table,
// until a '\0', or maxn.
// Return 0 on success, -1 on error.
int copyinstr(pgdir_t pgdir, char *dst, uint64_t srcva, uint64_t maxn)
{
    uint64_t n, va0, pa0;
    int got_null = 0;
    while(got_null == 0 && maxn > 0){
        va0 = PAGE_ROUNDDOWN(srcva);
        pa0 = (uint64_t)walkaddr(pgdir, va0);
        if(pa0 == 0)
            return -1;
        n = PAGE_SIZE - (srcva - va0);
        if(n > maxn)
            n = maxn;
        char *p = (char *) (pa0 + (srcva - va0));
        while(n > 0){
        if(*p == '\0'){
            *dst = '\0';
            got_null = 1;
            break;
        } else {
            *dst = *p;
        }
        --n;
        --maxn;
        p++;
        dst++;
        }
        srcva = va0 + PAGE_SIZE;
    }
    if(got_null){
        return 0;
    } else {
        return -1;
    }
}

int do_copy_from_user(void *dest, void *src, unsigned long nbytes)
{
    task_t *cur = task_current;
    if (dest && src) {
        if (copyin(cur->vmm->page_storage, dest, (uint64_t)src, nbytes) < 0) {
            errprintln("[page] do_copy_from_user: dest=%p src=%p nbytes=%d failed!", dest, src, nbytes);
            return -1;
        }
    }
    return 0;
}

int do_copy_from_user_str(char *dest, char *src, unsigned long maxn)
{
    task_t *cur = task_current;
    if (copyinstr(cur->vmm->page_storage, dest, (uint64_t)src, maxn) < 0) {
        errprintln("[page] do_copy_from_user: dest=%p src=%p nbytes=%d failed!", dest, src, maxn);
        return -1;
    }
    return strlen(dest);
}

int do_copy_to_user(void *dest, void *src, unsigned long nbytes)
{
    task_t *cur = task_current;
    if (dest && src) {
        if (copyout(cur->vmm->page_storage, (uint64_t)dest, src, nbytes) < 0) {
            errprintln("[page] do_copy_to_user: dest=%p src=%p nbytes=%d failed!", dest, src, nbytes);
            return -1;
        }
    }
    return 0;
}
