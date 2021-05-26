#include <arch/debug.h>
#include <arch/page.h>
#include <arch/time.h>
#include <arch/plic.h>
#include <arch/interrupt.h>
#include <arch/cpu.h>
#include <xbook/debug.h>
#include <k210_phymem.h>
#include <fpioa.h>
#include <dmac.h>

int arch_init(unsigned long hartid, unsigned long dtb_pa)
{	
    hartid_init(hartid);
    arch_debug_init();
    infoprint("welcome to xbook2!\n");
    physic_memory_init();
    page_init();
    cpu_init();
    interrupt_expection_init();
    trap_init();
    plic_init();
    
    #ifndef QEMU
    fpioa_pin_init();
    dmac_init();
    #endif 

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