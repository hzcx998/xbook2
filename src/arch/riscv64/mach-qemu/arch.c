#include <arch/debug.h>
#include <arch/page.h>
#include <xbook/debug.h>
#include <k210_qemu_phymem.h>

int arch_init()
{	
    arch_debug_init();
    infoprint("welcome to xbook2!\n");
    physic_memory_init();
    page_init();
    return 0;
}

int kernel_main()
{	
    infoprint("kernel_main start.\n");
    while (1)
    {
        /* code */
    }
    return 0;
}
