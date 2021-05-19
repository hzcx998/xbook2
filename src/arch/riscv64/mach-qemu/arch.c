#include <arch/debug.h>
#include <arch/page.h>
#include <arch/time.h>
#include <arch/plic.h>
#include <arch/interrupt.h>
#include <arch/cpu.h>
#include <xbook/debug.h>
#include <k210_qemu_phymem.h>

int arch_init()
{	
    arch_debug_init();
    infoprint("welcome to xbook2!\n");
    physic_memory_init();
    page_init();
    cpu_init();
    interrupt_expection_init();
    trap_init();
    plic_init();
    
    return 0;
}

#if 0
int kernel_main()
{	
    infoprint("kernel_main start.\n");
    while (1)
    {
        /* code */
    }
    return 0;
}
#endif