// Timer Interrupt handler

#include <xbook/spinlock.h>
#include <arch/sbi.h>
#include <arch/riscv.h>
#include <xbook/debug.h>

#define INTERVAL     (390000000 / 200) // timer interrupt interval

spinlock_t tickslock;
uint64_t ticks;

void timerinit() {
    spinlock_init(&tickslock);
}

void
set_next_timeout() {
    // There is a very strange bug,
    // if comment the `printf` line below
    // the timer will not work.

    // this bug seems to disappear automatically
    // printf("");
    sbi_set_timer(r_time() + INTERVAL);
}

void timer_tick() {
    keprintln("ticks: %d", ticks);
    spin_lock(&tickslock);
    ticks++;
    spin_unlock(&tickslock);
    set_next_timeout();
}
