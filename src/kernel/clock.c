#include <xbook/clock.h>
#include <assert.h>
#include <math.h>
#include <xbook/softirq.h>
#include <xbook/debug.h>
#include <xbook/softirq.h>
#include <xbook/task.h>
#include <xbook/schedule.h>
#include <xbook/timer.h>
#include <arch/interrupt.h>
#include <xbook/cpu.h>
#include <xbook/ktime.h>

volatile clock_t systicks;

/* 定时器软中断处理 */
void timer_softirq_handler(softirq_action_t *action)
{
	/* 改变系统时间 */
    if (systicks % HZ == 0) {  /* 1s更新一次 */
        /* 唤醒每秒时间工作 */
        update_ktime();
    }
    update_timers();
    update_alarms();
}

/* sched_softirq_handler - 调度程序软中断处理
 * @action: 中断行为
 * 
 * 在这里进行调度的抢占，也就是在这里面决定时间片轮训的时机。
 */
void sched_softirq_handler(softirq_action_t *action)
{
    
    #if 1
    task_t *current = current_task;
   
	/* 检测内核栈是否溢出 */
    ASSERT(current->stack_magic == TASK_STACK_MAGIC);
    /*if (current->stack_magic != TASK_STACK_MAGIC)
        dump_task(current);
    */
	/* 更新任务调度 */
	current->elapsed_ticks++;
	
    /* 需要进行调度的时候才会去调度 */
	if (current->ticks <= 0) {
		schedule();
	} else {
		current->ticks--;
	}
    #endif
}

/**
 * clock_handler - 时钟中断处理函数
 */
int clock_handler(unsigned long irq, unsigned long data)
{
    /* 改变ticks计数 */
	systicks++;

	/* 激活定时器软中断 */
	active_softirq(TIMER_SOFTIRQ);

	/* 激活调度器软中断 */
	active_softirq(SCHED_SOFTIRQ);
    return 0;
}

/**
 * sys_get_ticks - 获取系统的ticks
 * 
 */
clock_t sys_get_ticks()
{
    return systicks;
}

/**
 * 根据ticks延迟
 */
clock_t clock_delay_by_ticks(clock_t ticks)
{
    clock_t start = systicks;
    while (systicks - start < ticks)
        task_yeild();
    return ticks;
}

/**
 * 根据ticks延迟
 */
void mdelay(time_t msec)
{
    clock_t ticks = MSEC_TO_TICKS(msec);
    /* at least one ticket */
    if (!ticks)
        ticks = 1;

    clock_t start = systicks;
    while (sys_get_ticks() - start < ticks) {
        cpu_pause();
    }
}

/**
 * init_clock - 初始化时钟系统
 * 多任务的运行依赖于此
 */
void init_clock()
{
    systicks = 0;

    /* 初始化时钟硬件 */
    init_clock_hardware();

	/* 注册定时器软中断处理 */
	build_softirq(TIMER_SOFTIRQ, timer_softirq_handler);
	/* 注册定时器软中断处理 */
	build_softirq(SCHED_SOFTIRQ, sched_softirq_handler);
	/* 注册时钟中断并打开中断 */	
	if (register_irq(IRQ0_CLOCK, &clock_handler, IRQF_DISABLED, "clockirq", "kclock", 0))
        printk("register failed!\n");

}
