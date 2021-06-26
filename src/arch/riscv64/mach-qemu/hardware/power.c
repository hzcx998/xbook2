#include <xbook/power.h>
#include <xbook/debug.h>
#include <arch/sbi.h>
#include <arch/interrupt.h>
#include <arch/riscv.h>

void do_reboot()
{
    dbgprint("[power] do riscv reboot\n");
    dbgprint("not support reboot, shutdown now!\n");
    sbi_shutdown();
    panic("reboot failed!");
}

void do_halt()
{
    dbgprint("[power] do riscv halt\n");
    while (1)
        wait_for_interrupt();
    panic("halt failed!");
}

void do_poweroff()
{
    dbgprint("[power] do riscv poweroff\n");
    sbi_shutdown();
    panic("poweroff failed!");
}