#include <xbook/clock.h>
#include <assert.h>
#include <math.h>
#include <xbook/softirq.h>
#include <xbook/debug.h>
#if !defined(CLOCK_NO_SHCED)
#include <xbook/task.h>
#include <xbook/schedule.h>
#endif
#include <xbook/timer.h>
#include <xbook/hardirq.h>
#include <xbook/walltime.h>
#include <xbook/debug.h>
#include <arch/cpu.h>

volatile clock_t systicks;
volatile clock_t timer_ticks;

static void timer_softirq_handler(softirq_action_t *action)
{
    if (systicks % HZ == 0) {  /* 1s更新一次 */
        walltime_update_second();
    }
    timer_update_ticks();
    #if !defined(CLOCK_NO_SHCED)
    alarm_update_ticks();
    #endif
}

static void sched_softirq_handler(softirq_action_t *action)
{
    #if !defined(CLOCK_NO_SHCED)
    task_t *current = task_current;
    assert(current->stack_magic == TASK_STACK_MAGIC);
    current->elapsed_ticks++;
	if (current->ticks <= 0) {
		schedule();
	} else {
		current->ticks--;
	}
    #endif
}

int clock_handler(irqno_t irq, void *data)
{
	systicks++;
    timer_ticks++;
	softirq_active(TIMER_SOFTIRQ);
	softirq_active(SCHED_SOFTIRQ);
    return 0;
}

clock_t sys_get_ticks()
{
    return systicks;
}

clock_t clock_delay_by_ticks(clock_t ticks)
{
    clock_t start = systicks;
    #if !defined(CLOCK_NO_SHCED)
    while (systicks - start < ticks)
        task_yield();
    #else
    while (systicks - start < ticks);
    #endif
    return ticks;
}

void mdelay(time_t msec)
{
    clock_t ticks = MSEC_TO_TICKS(msec);
    if (!ticks)
        ticks = 1;
    clock_t start = systicks;
    while (sys_get_ticks() - start < ticks) {
        cpu_pause();
    }
}

void clock_init()
{
    timer_ticks = systicks = 0;
    softirq_build(TIMER_SOFTIRQ, timer_softirq_handler);
	softirq_build(SCHED_SOFTIRQ, sched_softirq_handler);
	clock_hardware_init();
    keprint(PRINT_INFO "[clock] init done\n");
}
