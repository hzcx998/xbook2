#include <arch/page.h>
#include <arch/mempool.h>
#include <xbook/debug.h>
#include <arch/riscv.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <k210_qemu_phymem.h>

/* 内核页目录表 */
pgdir_t kernel_pgdir;

extern char etext[];  // kernel.ld sets this to end of kernel code.

// Switch h/w page table register to the kernel's page table,
// and enable paging.
void page_enable()
{
    w_satp(MAKE_SATP(kernel_pgdir));
    sfence_vma();
    infoprint("page enable done.\n");
}
/*
初始化内核页表，启动分页机制
*/
void page_init()
{
    kernel_pgdir = (pgdir_t) page_alloc_normal(1);
    if (!kernel_pgdir)
        panic("alloc page for kernel page table failed!\n");
    dbgprint("kernel pgdir: %#p\n", kernel_pgdir);
    memset(kernel_pgdir, 0, PAGE_SIZE);

    /* 对内存地址进行虚拟地址映射 */
    // uart registers
    kvmmap(UART_V, UART, PAGE_SIZE, PAGE_ATTR_READ | PAGE_ATTR_WRITE);
  
    // CLINT
    kvmmap(CLINT_V, CLINT, 0x10000, PAGE_ATTR_READ | PAGE_ATTR_WRITE);
    keprintln("CLINT_V=%lx", CLINT_V);

    // PLIC
    keprintln("PLIC_V=%lx", PLIC_V);
    keprintln("VIRT_OFFSET=%lx", VIRT_OFFSET);
    
    
    kvmmap(PLIC_V, PLIC, 0x4000, PAGE_ATTR_READ | PAGE_ATTR_WRITE);
    kvmmap(PLIC_V + 0x200000, PLIC + 0x200000, 0x4000, PAGE_ATTR_READ | PAGE_ATTR_WRITE);

    /* 内核映射后，内核可以通过虚拟地址访问物理地址 */
    // map rustsbi
    //kvmmap(RUSTSBI_BASE, RUSTSBI_BASE, KERNBASE - RUSTSBI_BASE, PTE_R | PTE_X);
    // map kernel text executable and read-only.
    kvmmap(KERN_MEM_ADDR, KERN_MEM_ADDR, (uint64_t)etext - KERN_MEM_ADDR, PAGE_ATTR_READ | PAGE_ATTR_EXEC);
    // map kernel data and the physical RAM we'll make use of.
    kvmmap((uint64_t)etext, (uint64_t)etext, PHYSIC_MEM_TOP - (uint64_t)etext, PAGE_ATTR_READ | PAGE_ATTR_WRITE);

    /* 打开分页 */
    page_enable();

    uint64_t pa = addr_vir2phy(KERN_MEM_ADDR);
    dbgprint("addr_vir2phy: va=%x pa=%x\n", KERN_MEM_ADDR, pa);
    dbgprint("page_readable: va=%x %d\n", KERN_MEM_ADDR, page_readable(KERN_MEM_ADDR, PAGE_SIZE));
    dbgprint("page_writable: va=%x %d\n", KERN_MEM_ADDR, page_writable(KERN_MEM_ADDR, PAGE_SIZE));
    dbgprint("page_writable: va=%x %d\n", etext, page_writable((uint64_t)etext, PAGE_SIZE));

    vmprint(kernel_pgdir, 1);

    dbgprint("addr_vir2phy: va=%x pa=%x\n", PLIC_V, addr_vir2phy(PLIC_V));
}

// Return the address of the PTE in page table pgdir
// that corresponds to virtual address va.  If alloc!=0,
// create any required page-table pages.
//
// The risc-v Sv39 scheme has three levels of page-table
// pages. A page-table page contains 512 64-bit PTEs.
// A 64-bit virtual address is split into five fields:
//   39..63 -- must be zero.
//   30..38 -- 9 bits of level-2 index.
//   21..29 -- 9 bits of level-1 index.
//   12..20 -- 9 bits of level-0 index.
//    0..11 -- 12 bits of byte offset within the page.
pte_t *
walk(pgdir_t pgdir, uint64_t va, int alloc)
{
  
  if(va >= MAX_VIR_ADDR)
    panic("walk");

  for(int level = 2; level > 0; level--) {
    pte_t *pte = &pgdir[PX(level, va)];
    if(*pte & PAGE_ATTR_PRESENT) {
      pgdir = (pgdir_t)PTE2PA(*pte);
    } else {
      if(!alloc || (pgdir = (pgdir_t)page_alloc_normal(1)) == NULL)
        return NULL;      
      memset(pgdir, 0, PAGE_SIZE);
      *pte = PA2PTE(pgdir) | PAGE_ATTR_PRESENT;
    }
  }
  return &pgdir[PX(0, va)];
}


// add a mapping to the kernel page table.
// only used when booting.
// does not flush TLB or enable paging.
void
kvmmap(uint64_t va, uint64_t pa, uint64_t sz, int perm)
{
    dbgprint("kvmmap: va=%#lx pa=%#lx size=%#x perm=%#x\n", va, pa, sz, perm);
    if(mappages(kernel_pgdir, va, sz, pa, perm) != 0)
        panic("kvmmap");
}

// Create PTEs for virtual addresses starting at va that refer to
// physical addresses starting at pa. va and size might not
// be page-aligned. Returns 0 on success, -1 if walk() couldn't
// allocate a needed page-table page.
int
mappages(pgdir_t pgdir, uint64_t va, uint64_t size, uint64_t pa, int perm)
{
  uint64_t a, last;
  pte_t *pte;

  dbgprint("mappages: start=%lx end=%lx\n", va, va + size);
  
  a = PAGE_ROUNDDOWN(va);
  last = PAGE_ROUNDDOWN(va + size - 1);
  dbgprint("mappages: start=%lx end=%lx\n", a, last);
  for(;;){
    if((pte = walk(pgdir, a, 1)) == NULL)
      return -1;
    if(*pte & PAGE_ATTR_PRESENT)
      panic("remap: va=%lx pa=%lx\n", a, pa);
    *pte = PA2PTE(pa) | perm | PAGE_ATTR_PRESENT;
    
    if(a == last)
      break;
    a += PAGE_SIZE;
    pa += PAGE_SIZE;
  }
  return 0;
}

// Remove npages of mappings starting from va. va must be
// page-aligned. The mappings must exist.
// Optionally free the physical memory.
void
vmunmap(pgdir_t pgdir, uint64_t va, uint64_t npages, int do_free)
{
  uint64_t a;
  pte_t *pte;

  if((va % PAGE_SIZE) != 0)
    panic("vmunmap: not aligned");

  for(a = va; a < va + npages*PAGE_SIZE; a += PAGE_SIZE){
    if((pte = walk(pgdir, a, 0)) == 0)
      panic("vmunmap: walk");
    if((*pte & PAGE_ATTR_PRESENT) == 0)
      panic("vmunmap: not mapped");
    if(PTE_FLAGS(*pte) == PAGE_ATTR_PRESENT)
      panic("vmunmap: not a leaf");
    if(do_free){
      uint64_t pa = PTE2PA(*pte);
      page_free(pa);
    }
    *pte = 0;
  }
}


// Look up a virtual address, return the physical address,
// or 0 if not mapped.
// Can only be used to look up user pages.
uint64_t *
walkaddr(pgdir_t pgdir, uint64_t va)
{
  pte_t *pte;
  uint64_t pa;

  if(va >= MAX_VIR_ADDR)
    return NULL;

  pte = walk(pgdir, va, 0);
  if(pte == 0)
    return NULL;
  if((*pte & PAGE_ATTR_PRESENT) == 0)
    return NULL;
  if((*pte & PAGE_ATTR_USER) == 0)
    return NULL;
  pa = PTE2PA(*pte);
  return (uint64_t *) pa;
}

uint64_t
kwalkaddr(pgdir_t kpt, uint64_t va)
{
  uint64_t off = va % PAGE_SIZE;
  pte_t *pte;
  uint64_t pa;
  
  pte = walk(kpt, va, 0);
  if(pte == 0)
    panic("kvmpa");
  if((*pte & PAGE_ATTR_PRESENT) == 0)
    panic("kvmpa");
  pa = PTE2PA(*pte);
  return pa+off;
}

// translate a kernel virtual address to
// a physical address. only needed for
// addresses on the stack.
// assumes va is page aligned.
uint64_t
kvmpa(uint64_t va)
{
  return kwalkaddr(kernel_pgdir, va);
}

/**
 * 通过计算把虚拟地址转换为物理地址
 */
unsigned long addr_vir2phy(unsigned long vaddr)
{
    /* 从寄存器中读取页目录表地址 */
    pgdir_t pgdir = GET_CUR_PGDIR();
    keprintln("pgdir: %#p", pgdir);
    return kwalkaddr(pgdir, vaddr);
}

/* 释放所有非叶子页，也就是释放中间页 */
// Recursively free page-table pages.
// All leaf mappings must already have been removed.
void
freewalk(pgdir_t pgdir)
{
  // there are 2^9 = 512 PTEs in a page table.
  for(int i = 0; i < PAGE_TABLE_ENTRY_NR; i++){
    pte_t pte = pgdir[i];
    if((pte & PAGE_ATTR_PRESENT) && (pte & (PAGE_ATTR_READ|PAGE_ATTR_WRITE|PAGE_ATTR_EXEC)) == 0){
      // this PTE points to a lower-level page table.
      uint64_t child = PTE2PA(pte);
      freewalk((pgdir_t)child);
      pgdir[i] = 0;
    } else if(pte & PAGE_ATTR_PRESENT){
      panic("freewalk: leaf");
    }
  }
  page_free((unsigned long)pgdir);
}

bool page_readable(unsigned long vaddr, unsigned long nbytes)
{
    unsigned long addr = vaddr & PAGE_MASK;
    unsigned long count = PAGE_ALIGN(nbytes);
    pgdir_t pgdir = GET_CUR_PGDIR();
    while (count > 0) {
        pte_t *pte = walk(pgdir, vaddr, 0);
        if (!(*pte & PAGE_ATTR_PRESENT) || !(*pte & PAGE_ATTR_READ)) {
            return false;
        }
        addr += PAGE_SIZE;
        count -= PAGE_SIZE;
    }
    return true;
}

bool page_writable(unsigned long vaddr, unsigned long nbytes)
{
    unsigned long addr = vaddr & PAGE_MASK;
    unsigned long count = PAGE_ALIGN(nbytes);
    pgdir_t pgdir = GET_CUR_PGDIR();
    while (count > 0) {
        pte_t *pte = walk(pgdir, vaddr, 0);
        if (!(*pte & PAGE_ATTR_PRESENT) || !(*pte & PAGE_ATTR_WRITE)) {
            return false;
        }
        addr += PAGE_SIZE;
        count -= PAGE_SIZE;
    }
    return true;
}

int page_map_addr_fixed(unsigned long start, unsigned long addr, 
    unsigned long len, unsigned long prot)
{
    unsigned long flags;
    interrupt_save_and_disable(flags);
    unsigned long first = start & PAGE_MASK;
    len = PAGE_ALIGN(len);

    unsigned long attr = 0;
    
    if (prot & PROT_USER)
        attr |= PAGE_ATTR_USER;
    else
        attr |= PAGE_ATTR_SYSTEM;
    
    if (prot & PROT_WRITE)
        attr |= PAGE_ATTR_WRITE;
    else
        attr |= PAGE_ATTR_READ;

    if (prot & PROT_EXEC)
        attr |= PAGE_ATTR_EXEC;

    int retval = mappages(GET_CUR_PGDIR(), first, len, addr & PAGE_MASK, attr);
    interrupt_restore_state(flags);
	return retval;
}

int page_unmap_addr(unsigned long vaddr, unsigned long len)
{
    if (!len)
		return -1;
    unsigned long flags;
    interrupt_save_and_disable(flags);
	len = PAGE_ALIGN(len);
    vmunmap(GET_CUR_PGDIR(), vaddr & PAGE_MASK, len / PAGE_SIZE, 1);
    interrupt_restore_state(flags);
	return 0;
}

int page_unmap_addr_safe(unsigned long start, unsigned long len, char fixed)
{
    if (!len)
		return -1;
    unsigned long flags;
    interrupt_save_and_disable(flags);
	len = PAGE_ALIGN(len);
    vmunmap(GET_CUR_PGDIR(), start & PAGE_MASK, len / PAGE_SIZE, !fixed);   /* 不是固定才释放 */
    interrupt_restore_state(flags);
	return 0;
}

int page_map_addr(unsigned long start, unsigned long len, unsigned long prot)
{
    unsigned long flags;
    interrupt_save_and_disable(flags);
    unsigned long vaddr = (unsigned long )start & PAGE_MASK;
    len = PAGE_ALIGN(len);
    unsigned long pages = PAGE_ROUNDUP(len);
    unsigned long page_idx = 0;
    unsigned long page_addr;
    unsigned long attr = 0;
    
    if (prot & PROT_USER)
        attr |= PAGE_ATTR_USER;
    else
        attr |= PAGE_ATTR_SYSTEM;
    
    if (prot & PROT_WRITE)
        attr |= PAGE_ATTR_WRITE;
    else
        attr |= PAGE_ATTR_WRITE;

    if (prot & PROT_EXEC)
        attr |= PAGE_ATTR_EXEC;

    pgdir_t pgdir = GET_CUR_PGDIR();
    int retval = -1;
    while (page_idx < pages) {
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
int page_map_addr_safe(unsigned long start, unsigned long len, unsigned long prot)
{
    unsigned long flags;
    interrupt_save_and_disable(flags);
    unsigned long vaddr = (unsigned long )start & PAGE_MASK;
    len = PAGE_ALIGN(len);
    unsigned long pages = PAGE_ROUNDUP(len);
    unsigned long page_idx = 0;
    unsigned long page_addr;
    unsigned long attr = 0;
    
    if (prot & PROT_USER)
        attr |= PAGE_ATTR_USER;
    else
        attr |= PAGE_ATTR_SYSTEM;
    
    if (prot & PROT_WRITE)
        attr |= PAGE_ATTR_WRITE;
    else
        attr |= PAGE_ATTR_WRITE;

    if (prot & PROT_EXEC)
        attr |= PAGE_ATTR_EXEC;

    pgdir_t pgdir = GET_CUR_PGDIR();
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

unsigned long *kern_page_dir_copy_to()
{
    unsigned long flags;
    interrupt_save_and_disable(flags);
    unsigned long page = page_alloc_normal(1);
    unsigned int *vaddr = (unsigned int *)kern_phy_addr2vir_addr(page);
    
    memset(vaddr, 0, PAGE_SIZE);
    memcpy((void *)vaddr, (void *)kernel_pgdir, PAGE_SIZE);
    interrupt_restore_state(flags);
    return (unsigned long *)vaddr;
}

// TODO: add page_do_fault

/**
 * 打印页目录表中的页表映射关系
 * @pgdir: 要打印的页目录表
 * @level: 最高打印的等级：[0,1,2]
 */
void vmprint(pgdir_t pgdir, int level)
{
  const int capacity = 512;
  dbgprint("page table %p\n", pgdir);
  pte_t *pte;
  for (pte = (pte_t *) pgdir; pte < pgdir + capacity; pte++) {
    if (*pte & PAGE_ATTR_PRESENT && level >= 0)
    {
      pgdir_t pt2 = (pgdir_t) PTE2PA(*pte); 
      dbgprint("..%d: pte %p pa %p\n", pte - pgdir, *pte, pt2);
      pte_t *pte2;
      for (pte2 = (pte_t *) pt2; pte2 < pt2 + capacity; pte2++) {
        if (*pte2 & PAGE_ATTR_PRESENT && level >= 1)
        {
          pgdir_t pt3 = (pgdir_t) PTE2PA(*pte2);
          dbgprint(".. ..%d: pte %p pa %p\n", pte2 - pt2, *pte2, pt3);
          pte_t *pte3;
          for (pte3 = (pte_t *) pt3; pte3 < pt3 + capacity; pte3++)
            if (*pte3 & PAGE_ATTR_PRESENT && level >= 2)
              dbgprint(".. .. ..%d: pte %p pa %p\n", pte3 - pt3, *pte3, PTE2PA(*pte3));
        }
      }
    }
  }
  return;
}