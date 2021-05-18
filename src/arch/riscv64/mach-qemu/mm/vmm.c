#include <arch/vmm.h>
#include <arch/page.h>
#include <arch/riscv.h>

extern pgdir_t kernel_pgdir;

void vmm_active_kernel()
{
    /* 激活内核页表 */
    w_satp(MAKE_SATP(kernel_pgdir));
    sfence_vma();
}

void vmm_active_user(unsigned long page)
{
    /* 激活用户页表 */
    w_satp(MAKE_SATP(page));
    sfence_vma();
}
