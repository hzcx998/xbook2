#include <arch/vmm.h>
#include <arch/page.h>
#include <arch/riscv.h>
#include <xbook/debug.h>

extern pgdir_t kernel_pgdir;

// #define DEBUG_ARCH_VMM

void vmm_active_kernel()
{
    #ifdef DEBUG_ARCH_VMM
    keprintln("[vmm] vmm_active_kernel");
    #endif
    /* 如果时同一个页表就不改变，这样减少对页表的刷新，提高效率 */
    if (r_satp() == MAKE_SATP(kernel_pgdir))
        return;
    /* 激活内核页表 */
    w_satp(MAKE_SATP(kernel_pgdir));
    sfence_vma();
}

void vmm_active_user(unsigned long page)
{
    #ifdef DEBUG_ARCH_VMM
    keprintln("[vmm] vmm_active_user");
    #endif
    if (r_satp() == MAKE_SATP(kernel_pgdir))
        return;
    /* 激活用户页表 */
    w_satp(MAKE_SATP(page));
    sfence_vma();
}
