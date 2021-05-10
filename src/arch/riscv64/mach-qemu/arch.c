#include <arch/debug.h>
#include <xbook/debug.h>

int arch_init()
{	
    arch_debug_init();
    infoprint("welcome to xbook2!\n");

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
