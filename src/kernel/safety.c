#include <xbook/safety.h>
#include <xbook/kernel.h>
#include <xbook/schedule.h>
#include <xbook/vmm.h>
#include <arch/page.h>
#include <string.h>

int safety_check_range(void *src, unsigned long nbytes)
{
    unsigned long addr;
    addr = (unsigned long) src;
    #if defined(SAFETY_TINY)
    if (!((addr >= USER_VMM_BASE_ADDR) && (addr + nbytes < USER_VMM_TOP_ADDR))) {
        return -1;
    }
    #else
    if (task_current->vmm && !((addr >= USER_VMM_BASE_ADDR) && (addr + nbytes < USER_VMM_TOP_ADDR))) {
        return -1;
    }
    #endif
    return 0;
}

int mem_copy_from_user(void *dest, void *src, unsigned long nbytes)
{
    if (safety_check_range(src, nbytes) < 0)
        return -1;
    if (!page_readable((unsigned long) src, nbytes))
        return -1;
    if (dest && src)
        memcpy(dest, src, nbytes);
    return 0;
}

int mem_copy_to_user(void *dest, void *src, unsigned long nbytes)
{
    if (safety_check_range(dest, nbytes) < 0)
        return -1;
    if (!page_writable((unsigned long) dest, nbytes))
        return -1;  
    if (dest && src)
        memcpy(dest, src, nbytes);
    return 0;
}
