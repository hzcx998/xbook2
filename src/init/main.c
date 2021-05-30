#include <xbook/memcache.h>
#include <xbook/debug.h>
#include <xbook/hardirq.h>
#include <xbook/softirq.h>
#include <xbook/clock.h>
#include <xbook/virmem.h>
#include <xbook/walltime.h>
#include <xbook/timer.h>
#include <xbook/tests.h>
#include <xbook/task.h>
#include <xbook/schedule.h>
#include <xbook/driver.h>
#include <xbook/fifo.h>
#include <xbook/initcall.h>
#include <xbook/fs.h>
#include <xbook/syscall.h>
#include <xbook/account.h>
#include <xbook/sharemem.h>
#include <xbook/msgqueue.h>
#include <xbook/sem.h>
#include <xbook/tests.h>
#include <xbook/portcomm.h>
#ifdef CONFIG_ACCOUNT 
#include <xbook/account.h>
#endif

int kernel_main(void)
{
    keprint(PRINT_INFO "welcome to xbook kernel.\n");
    memory_overview();
    mem_caches_init();
    vir_mem_init();
    irq_description_init();
    softirq_init();
    schedule_init();
    tasks_init();
    syscall_init();
    walltime_init();
    clock_init();
    timers_init();
    fifo_init();
    share_mem_init();
    msg_queue_init();
    sem_init();
    interrupt_enable();
    driver_framewrok_init();
    initcalls_exec();
    drivers_print();
    file_system_init();
    #ifdef CONFIG_ACCOUNT 
    account_manager_init();
    #endif
    port_comm_init();
    kernel_test_init();
    memory_overview();
    task_start_user();
    return 0;    
}
