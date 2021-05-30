// Timer Interrupt handler

#include <xbook/spinlock.h>
#include <arch/sbi.h>
#include <arch/riscv.h>
#include <arch/time.h>
#include <xbook/debug.h>

/* 值越小，间隔就越小，产生中断的速度就越快。
参考值：
390000000 / 200: 1s产生5次
390000000 / 400: 1s产生10次
390000000 / 800: 1s产生20次
390000000 / 4000：1s产生100次
*/
#define TIMER_INTR_INTERVAL     (390000000 / (40 * HZ)) // timer interrupt interval

uint64_t ticks;

void timer_interrupt_init() {
    // set next clock
    timer_interrupt_set_next_timeout();
}

void timer_interrupt_set_next_timeout()
{
    sbi_set_timer(time_reg_read() + TIMER_INTR_INTERVAL);
}
