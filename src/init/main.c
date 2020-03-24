#include <arch/interrupt.h>
#include <xbook/memcache.h>
#include <xbook/debug.h>
#include <xbook/softirq.h>
#include <xbook/clock.h>
#include <xbook/vmarea.h>
#include <xbook/task.h>
#include <xbook/unit.h>

int kernel_main(void)
{
    printk(KERN_INFO "welcom to xbook kernel.\n");
    /* init memory cache for kernel */
    init_mem_caches();
    init_vmarea();
    /* init irq description */
    init_irq_description();
    /* init softirq */
    init_softirq();
    
    init_tasks();
    /* init clock */
    init_clock();
    /* init unit drivers */
    init_unit();

    kernel_pause();
    return 0;
}

