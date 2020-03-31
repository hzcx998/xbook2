#include <arch/interrupt.h>
#include <xbook/memcache.h>
#include <xbook/debug.h>
#include <xbook/softirq.h>
#include <xbook/clock.h>
#include <xbook/vmarea.h>
#include <xbook/task.h>
#include <xbook/unit.h>
#include <xbook/rawblock.h>
#include <xbook/schedule.h>

void delay(int t)
{
    int i, j;
    for (i = 0; i < 100 * t; i++) {
        for (j = 0; j < 1000; j++);
    }
}

void ktask_main(void *arg)
{
    //printk("ktask_main running...\n");

    int ticks = 0;
    while (1)
    {
        ticks++;
        //printk("A:%x\n", ticks);
        delay(2);
    }
    
}

void ktask2_main(void *arg)
{
    printk("ktask2_main running...\n");

    int ticks = 0X1000;
    while (1)
    {
        ticks++;
        printk("B:%x\n", ticks);
        delay(1);
    }
    
}
void ktask3_main(void *arg)
{
    printk("ktask3_main running...\n");

    int ticks = 0X1000;
    while (1)
    {
        ticks++;
        printk("C:%x\n", ticks);
        delay(1);
    }
    
}

extern trap_frame_t * current_trap_frame;
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
    init_clock();
    
    kthread_start("idle", TASK_PRIO_IDLE, ktask_main, NULL);
    /*task_t *ktask2 = kthread_start("ktask2", TASK_PRIO_USER, ktask2_main, NULL);
    task_t *ktask3 = kthread_start("ktask3", TASK_PRIO_USER, ktask3_main, NULL);
    */
    
    /* init unit drivers */
    init_unit();

    /* init raw block */
    init_raw_block();

    char *argv[3] = {"test", "arg2", 0};
    process_create("test", argv);
    launch_task();
    while (1);
    /*char *argv1[3] = {"test", "arg3", 0};
    process_create("test", argv1);
    */
    kernel_pause();
    return 0;
}

