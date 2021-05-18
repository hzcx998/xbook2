#include <xbook/memcache.h>
#include <xbook/debug.h>
#include <xbook/hardirq.h>
#include <xbook/softirq.h>
#include <xbook/clock.h>
#include <xbook/virmem.h>
#include <xbook/walltime.h>
#include <xbook/timer.h>
#include <xbook/tests.h>
// #include <arch/proc.h>
#include <xbook/task.h>
#include <xbook/schedule.h>

// 临时测试
// #include <arch/proc.h>

int kernel_main(void)
{
    keprint(PRINT_INFO "welcome to xbook kernel.\n");
    mem_caches_init();
    vir_mem_init();
    irq_description_init();
    softirq_init();
    
    schedule_init();
    // procinit();
    tasks_init();
    walltime_init();
    clock_init();
    timers_init();
    interrupt_enable();

    kernel_test_init();

    //task_start_other();
    task_start_user();
    
    /*
    clock_init();
    timers_init();
    interrupt_enable();
    
    driver_framewrok_init();
    initcalls_exec();
    
    file_system_init();*/
    while (1) {
        /* code */
    }
    return 0;    
}
