#include <stdint.h>
#include <arch/riscv.h>
#include <xbook/debug.h>
#include <arch/interrupt.h>
#include <arch/riscv.h>
#include <xbook/schedule.h>
#include <xbook/syscall.h>
#include <xbook/exception.h>

// #define DEBUG_TRAP

// in kernel_trap_entry.S, calls do_kernel_trap().
extern void kernel_trap_entry();

extern char trampoline[], uservec[], userret[];

// set up to take exceptions and traps while in the kernel.
void trap_init(void)
{
    // register interrupt handler
    stvec_write((uint64_t)kernel_trap_entry);
    
    // enable supervisor-mode timer interrupts.
    sie_write(sie_read() | SIE_SEIE | SIE_SSIE | SIE_STIE);
}

// interrupts and exceptions from kernel code go here via kernel_trap_entry,
// on whatever the current kernel stack is.
void do_kernel_trap(trap_frame_t *frame)
{
#ifdef DEBUG_TRAP
    dbgprintln("trap frame addr:%p", frame);
#endif
    uint64_t sepc = sepc_read();
    uint64_t sstatus = sstatus_read();

    if((sstatus & SSTATUS_SPP) == 0)
        panic("do_kernel_trap: not from supervisor mode");
    if(interrupt_enabled() != 0)
        panic("do_kernel_trap: interrupts enabled");

    /* 对中断函数进行派发 */
    interrupt_dispatch(frame);

    exception_check(frame);

    // the interrupt may have caused some traps to occur,
    // so restore trap registers for use by kerneltraps.S's sepc instruction.
    sepc_write(sepc);
    sstatus_write(sstatus);
}

//
// handle an interrupt, exception, or system call from user space.
// called from trampoline.S
//
void usertrap(void)
{
    if((sstatus_read() & SSTATUS_SPP) != 0)
        panic("usertrap: not from user mode");

    // send interrupts and exceptions to kerneltrap(),
    // since we're now in the kernel.
    stvec_write((uint64_t)kernel_trap_entry);

    task_t *cur = task_current;
    
    // save user program counter.
    cur->trapframe->epc = sepc_read();
    
    if(scause_read() == 8){
        // system call
        // sepc points to the ecall instruction,
        // but we want to return to the next instruction.
        cur->trapframe->epc += 4;
        // an interrupt will change sstatus &c registers,
        // so don't enable until done with those registers.
        interrupt_enable();
        syscall_dispatch(cur->trapframe);
    } else {
        interrupt_dispatch(cur->trapframe);
    }
    exception_check(cur->trapframe);
    usertrapret();
}

//
// return to user space
//
void usertrapret(void)
{
    task_t *cur = task_current;

    // we're about to switch the destination of traps from
    // kerneltrap() to usertrap(), so turn off interrupts until
    // we're back in user space, where usertrap() is correct.
    interrupt_disable();
    // send syscalls, interrupts, and exceptions to trampoline.S
    stvec_write(TRAMPOLINE + (uservec - trampoline));

    // set up trapframe values that uservec will need when
    // the process next re-enters the kernel.
    cur->trapframe->kernel_satp = satp_read();         // kernel page table
    cur->trapframe->kernel_sp = (uint64_t)(cur->kstack + PAGE_SIZE); // process's kernel stack
    cur->trapframe->kernel_trap = (uint64_t)usertrap;
    cur->trapframe->kernel_hartid = tp_reg_read();         // hartid for cpuid()

    // set up the registers that trampoline.S's sret will use
    // to get to user space.

    // set S Previous Privilege mode to User.
    unsigned long x = sstatus_read();
    x &= ~SSTATUS_SPP; // clear SPP to 0 for user mode
    x |= SSTATUS_SPIE; // enable interrupts in user mode
    sstatus_write(x);

    // set S Exception Program Counter to the saved user pc.
    sepc_write(cur->trapframe->epc);

    // tell trampoline.S the user page table to switch to.
    uint64_t satp = 0;
    if (cur->vmm) {
        satp = MAKE_SATP((unsigned long)cur->vmm->page_storage);
    } else {
        panic("usertrapret: no vmm");
    }

    // jump to trampoline.S at the top of memory, which 
    // switches to the user page table, restores user registers,
    // and switches to user mode with sret.
    uint64_t fn = TRAMPOLINE + (userret - trampoline);
    ((void (*)(trap_frame_t *,uint64_t))fn)(cur->trapframe, satp);
}

void forkret()
{
  #ifdef DEBUG_TRAP
  task_t *cur = task_current;
  keprintln("[fork] task %s pid=%d ppid=%d return to user", cur->name, cur->pid, cur->parent_pid);
  trap_frame_dump(cur->trapframe);
  #endif
  usertrapret();
}
