#include <stdint.h>
#include <arch/riscv.h>
#include <xbook/debug.h>
#include <arch/interrupt.h>
#include <arch/riscv.h>
#include <xbook/schedule.h>
#include <xbook/syscall.h>

// in kernel_trap_entry.S, calls do_kernel_trap().
extern void kernel_trap_entry();

extern char trampoline[], uservec[], userret[];

void usertrapret(void);

// set up to take exceptions and traps while in the kernel.
void trap_init(void)
{
    // register interrupt handler
    w_stvec((uint64_t)kernel_trap_entry);
    
    // enable supervisor-mode timer interrupts.
    w_sie(r_sie() | SIE_SEIE | SIE_SSIE | SIE_STIE);
}

// interrupts and exceptions from kernel code go here via kernel_trap_entry,
// on whatever the current kernel stack is.
void do_kernel_trap(trap_frame_t *frame)
{
    //dbgprintln("trap frame addr:%p", frame);
    //trap_frame_dump(frame);

    //keprint("run in kerntrap\n");

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

//
// handle an interrupt, exception, or system call from user space.
// called from trampoline.S
//
void
usertrap(void)
{
  keprint("run in usertrap\n");

  if((r_sstatus() & SSTATUS_SPP) != 0)
    panic("usertrap: not from user mode");

  // send interrupts and exceptions to kerneltrap(),
  // since we're now in the kernel.
  w_stvec((uint64_t)kernel_trap_entry);

  task_t *p = task_current;
  
  // save user program counter.
  p->trapframe->epc = r_sepc();
  
  if(r_scause() == 8){
    dbgprintln("syscall intr!");
    // system call
    // sepc points to the ecall instruction,
    // but we want to return to the next instruction.
    p->trapframe->epc += 4;
    // an interrupt will change sstatus &c registers,
    // so don't enable until done with those registers.
    interrupt_enable();
    // TODO: call syscall
    // syscall();
    syscall_dispatch(p->trapframe);
  } 
  else {
    interrupt_dispatch(p->trapframe);
    // panic("## not syscall trap!!");
  }

  usertrapret();
}

//
// return to user space
//
void
usertrapret(void)
{
  task_t *p = task_current;

  // we're about to switch the destination of traps from
  // kerneltrap() to usertrap(), so turn off interrupts until
  // we're back in user space, where usertrap() is correct.
  interrupt_disable();
  //interrupt_enable();
  keprint("intr enable %d\n", interrupt_enabled());
  // send syscalls, interrupts, and exceptions to trampoline.S
  w_stvec(TRAMPOLINE + (uservec - trampoline));

  // set up trapframe values that uservec will need when
  // the process next re-enters the kernel.
  p->trapframe->kernel_satp = r_satp();         // kernel page table
  p->trapframe->kernel_sp = (uint64_t)(p->kstack + PAGE_SIZE); // process's kernel stack
  p->trapframe->kernel_trap = (uint64_t)usertrap;
  p->trapframe->kernel_hartid = r_tp();         // hartid for cpuid()

  // set up the registers that trampoline.S's sret will use
  // to get to user space.

  // set S Previous Privilege mode to User.
  unsigned long x = r_sstatus();
  x &= ~SSTATUS_SPP; // clear SPP to 0 for user mode
  x |= SSTATUS_SPIE; // enable interrupts in user mode
  w_sstatus(x);

  // set S Exception Program Counter to the saved user pc.
  w_sepc(p->trapframe->epc);

  // tell trampoline.S the user page table to switch to.
  // printf("[usertrapret]p->pagetable: %p\n", p->pagetable);
  // uint64_t satp = MAKE_SATP(p->pagetable);
    uint64_t satp = 0;
    if (p->vmm) {
        satp = MAKE_SATP((unsigned long)p->vmm->page_storage);
        //keprintln("usertrapret: have vmm %lx", satp);
    } else {
        panic("usertrapret: no vmm");
    }

  // jump to trampoline.S at the top of memory, which 
  // switches to the user page table, restores user registers,
  // and switches to user mode with sret.
  uint64_t fn = TRAMPOLINE + (userret - trampoline);
  ((void (*)(uint64_t,uint64_t))fn)(TRAPFRAME, satp);
}