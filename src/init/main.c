#include <arch/interrupt.h>
#include <xbook/memcache.h>
#include <xbook/debug.h>
#include <xbook/softirq.h>
#include <xbook/clock.h>

int kernel_main(void)
{
    printk("welcome to kernel main!\n");
    /* init memory cache for kernel */
    init_mem_caches();
    /* init irq description */
    init_irq_description();
    /* init softirq */
    init_softirq();
    /* init clock */
    init_clock();
    return 0;
}

