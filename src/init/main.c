#include <arch/interrupt.h>
#include <xbook/memcache.h>
#include <xbook/debug.h>
#include <xbook/softirq.h>
#include <xbook/clock.h>
#include <xbook/vmarea.h>
#include <xbook/task.h>
#include <xbook/unit.h>
#include <xbook/rawblock.h>

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

    /* init raw block */
    init_raw_block();

    printk("process create.\n");
    char *argv[3] = {"test", "arg2", 0};
    process_create("test", argv);

    kernel_pause();
    return 0;
}

