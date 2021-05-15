#include <arch/debug.h>
#include <arch/page.h>
#include <arch/time.h>
#include <arch/plic.h>
#include <arch/interrupt.h>
#include <xbook/debug.h>
#include <k210_qemu_phymem.h>

int arch_init()
{	
    arch_debug_init();
    infoprint("welcome to xbook2!\n");
    physic_memory_init();
    page_init();
    interrupt_expection_init();
    timer_interrupt_init(); // 初始化内核中的clock后，需要将本行删除
    trap_init();
    plic_init();
    interrupt_enable(); // NOTE: 本行需要在初始化多任务后删除
    #if 0
    char *v = (char *)0;
    *v =1;
    #endif
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
