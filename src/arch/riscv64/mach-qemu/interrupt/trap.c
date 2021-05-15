#include <stdint.h>
#include <arch/riscv.h>
#include <xbook/debug.h>
#include <arch/interrupt.h>

// in kernel_trap_entry.S, calls do_kernel_trap().
extern void kernel_trap_entry();

// set up to take exceptions and traps while in the kernel.
void trap_init(void)
{
    // register interrupt handler
    w_stvec((uint64_t)kernel_trap_entry);
}

// interrupts and exceptions from kernel code go here via kernel_trap_entry,
// on whatever the current kernel stack is.
void do_kernel_trap(trap_frame_t *frame)
{
    dbgprintln("trap frame addr:%p", frame);
    //trap_frame_dump(frame);

    int which_dev = 0;
    uint64_t sepc = r_sepc();
    uint64_t sstatus = r_sstatus();
    // uint64_t scause = r_scause();
  
    if((sstatus & SSTATUS_SPP) == 0)
        panic("do_kernel_trap: not from supervisor mode");
    if(interrupt_enabled() != 0)
        panic("do_kernel_trap: interrupts enabled");

    /* 对中断函数进行派发 */
    interrupt_dispatch(frame);

    // the yield() may have caused some traps to occur,
    // so restore trap registers for use by kernel_trap_entry.S's sepc instruction.
    w_sepc(sepc);
    w_sstatus(sstatus);
}
