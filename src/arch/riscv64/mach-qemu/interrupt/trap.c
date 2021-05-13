#include <stdint.h>
#include <arch/riscv.h>
#include <xbook/debug.h>
#include <arch/interrupt.h>

// in kernelvec.S, calls kerneltrap().
extern void kernelvec();

// set up to take exceptions and traps while in the kernel.
void
trapinithart(void)
{
  w_stvec((uint64_t)kernelvec);
  w_sstatus(r_sstatus() | SSTATUS_SIE);
  // enable supervisor-mode timer interrupts.
  w_sie(r_sie() | SIE_SEIE | SIE_SSIE | SIE_STIE);
  set_next_timeout();
}

// Check if it's an external/software interrupt, 
// and handle it. 
// returns  2 if timer interrupt, 
//          1 if other device, 
//          0 if not recognized. 
static int devintr(void) {
	uint64_t scause = r_scause();

	// handle external interrupt 
	if ((0x8000000000000000L & scause) && 9 == (scause & 0xff)) 
	{
		keprintln("external interrupt");
		return 1;
	}
	else if (0x8000000000000005L == scause) {
		timer_tick();
		return 2;
	}
	else { return 0;}
}

// interrupts and exceptions from kernel code go here via kernelvec,
// on whatever the current kernel stack is.
void 
kerneltrap() {
  int which_dev = 0;
  uint64_t sepc = r_sepc();
  uint64_t sstatus = r_sstatus();
  uint64_t scause = r_scause();
  
  if((sstatus & SSTATUS_SPP) == 0)
    panic("kerneltrap: not from supervisor mode");
  if(intr_get() != 0)
    panic("kerneltrap: interrupts enabled");

  if((which_dev = devintr()) == 0){
    keprint("\nscause %p\n", scause);
    keprint("sepc=%p stval=%p hart=%d\n", r_sepc(), r_stval(), r_tp());
    panic("kerneltrap");
  }
  keprint("which_dev: %d\n", which_dev);
  
  // the yield() may have caused some traps to occur,
  // so restore trap registers for use by kernelvec.S's sepc instruction.
  w_sepc(sepc);
  w_sstatus(sstatus);
}
