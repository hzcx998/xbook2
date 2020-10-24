#include <xbook/memcache.h>
#include <xbook/debug.h>
#include <xbook/softirq.h>
#include <xbook/clock.h>
#include <xbook/vmarea.h>
#include <xbook/task.h>
#include <xbook/rawblock.h>
#include <xbook/schedule.h>
#include <xbook/sharemem.h>
#include <xbook/msgqueue.h>
#include <xbook/sem.h>
#include <xbook/syscall.h>
#include <xbook/fifo.h>
#include <xbook/driver.h>
#include <xbook/ktime.h>
#include <xbook/srvcall.h>
#include <xbook/fs.h>
#include <xbook/net.h>
#include <xbook/gui.h>
#include <xbook/timer.h>

int kernel_main(void)
{
    printk(KERN_INFO "welcome to xbook kernel.\n");
    
    /* init memory cache for kernel */
    init_mem_caches();
    init_vmarea();
    
    /* init irq description */
    init_irq_description();
    /* init softirq */
    init_softirq();
    
    /* init ipc */
    init_share_mem();
    init_msg_queue();
    init_sem();
    init_fifo();

    init_syscall();
    init_srvcall();
    
    init_ktime();

    init_tasks();
    
    init_clock();
    
    init_timer_system();

    /* enable interrupt */
    enable_intr();

    init_driver_arch();
    
    /* init fs */
    init_fs();

    init_gui();
    // init_net();
    //spin("test");
    start_user();

    return 0;    
}
