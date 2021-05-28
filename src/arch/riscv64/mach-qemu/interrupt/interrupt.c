#include <arch/interrupt.h>
#include <arch/riscv.h>
#include <arch/page.h>
#include <xbook/debug.h>
#include <xbook/clock.h>
#include <xbook/softirq.h>
#include <xbook/exception.h>
#include <xbook/schedule.h>
#include <stddef.h>

extern int interrupt_do_irq(trap_frame_t *frame);

interrupt_handler_t interrupt_handlers[MAX_INTERRUPT_NR];
char* interrupt_names[MAX_INTERRUPT_NR];		     

void interrupt_disable(void)
{
    w_sstatus(r_sstatus() & ~SSTATUS_SIE);
}

void interrupt_enable(void)
{
    w_sstatus(r_sstatus() | SSTATUS_SIE);
}

static void interrupt_general_handler(trap_frame_t *frame) 
{
	//dbgprintln("[interrupt] into general handler!");
    uint64_t scause = r_scause();   // 获取中断产生的原因
    /* 来自设备、定时器中断 */
    if (scause & SCAUSE_INTERRUPT) {
        dbgprintln("[interrupt] external interrupt %d occur!", scause);
    } else {    /* 来自异常 */
        int expcode = scause & 0xff;
        /* 处理页故障 */
        if (expcode == EP_INSTRUCTION_PAGE_FAULT || 
            expcode == EP_LOAD_PAGE_FAULT || 
            expcode == EP_STORE_PAGE_FAULT) {
            if (page_do_fault(frame, (scause & SSTATUS_SPP) == 0, expcode) < 0);
                trap_frame_dump(frame);
            return;
        }
        /* 其它异常 */
        if (task_init_done) {
            task_t *cur = task_current;
            keprint("[exception] task name:%s, pid:%d\n", cur->name, cur->pid);
        }
        keprint("[exception] scause %p\n", scause);
        keprint("[exception] sepc=%p stval=%p hart=%d\n", r_sepc(), r_stval(), r_tp());
        keprint("[exception] name: %s \n", interrupt_names[expcode]);
        trap_frame_dump(frame);
        if((scause & SSTATUS_SPP) != 0) {  // from kernel
            dbgprintln("[exception] exception %d from kernel!", expcode);
            panic("exception in kernel :(");
        } else {    // from user
            dbgprintln("[exception] exception %d from user!", expcode);
            panic("exception in user :(");
        }
    }
}

void interrupt_expection_init(void)
{
	int i;
   	for (i = 0; i < MAX_INTERRUPT_NR; i++) {
      	interrupt_handlers[i] = interrupt_general_handler;		    
      	interrupt_names[i] = "unknown";    
   	}
    interrupt_names[0] = "Instruction address misaligned";
    interrupt_names[1] = "Instruction address fault";
    interrupt_names[2] = "Illegal instruction";
    interrupt_names[3] = "Breakpoint";
    interrupt_names[4] = "Load address misaligned";
    interrupt_names[5] = "Load address fault";
    interrupt_names[6] = "Store address misaligned";
    interrupt_names[7] = "Store address fault";
    interrupt_names[8] = "Environment call from U-mode";
    interrupt_names[9] = "Environment call from S-mode";
    interrupt_names[11] = "Environment call from M-mode";
    interrupt_names[12] = "Instruction page fault";
    interrupt_names[13] = "Load page fault";
    interrupt_names[15] = "Store page fault";
}

int irq_register_handler(int irq, interrupt_handler_t function) 
{
	if (irq < 0 || irq >= IRQ_MAX_NR) {
		return -1;
	}
   	interrupt_handlers[IRQ_OFFSET + irq] = function; 
    return 0;
}

int irq_unregister_handler(int irq)
{
	if (irq < 0 || irq > IRQ_MAX_NR) {
		return -1;
	}
	interrupt_handlers[IRQ_OFFSET + irq] = interrupt_general_handler; 
    return 0;
}

void trap_frame_dump(trap_frame_t *frame)
{
    dbgprintln("[dump]: trap_frame_dump");
    dbgprint("a0: %p\t", frame->a0);
    dbgprint("a1: %p\t", frame->a1);
    dbgprint("a2: %p\t", frame->a2);
    dbgprint("a3: %p\n", frame->a3);
    dbgprint("a4: %p\t", frame->a4);
    dbgprint("a5: %p\t", frame->a5);
    dbgprint("a6: %p\t", frame->a6);
    dbgprint("a7: %p\n", frame->a7);
    dbgprint("t0: %p\t", frame->t0);
    dbgprint("t1: %p\t", frame->t1);
    dbgprint("t2: %p\t", frame->t2);
    dbgprint("t3: %p\n", frame->t3);
    dbgprint("t4: %p\t", frame->t4);
    dbgprint("t5: %p\t", frame->t5);
    dbgprint("t6: %p\t", frame->t6);
    dbgprint("s0: %p\n", frame->s0);
    dbgprint("s1: %p\t", frame->s1);
    dbgprint("s2: %p\t", frame->s2);
    dbgprint("s3: %p\t", frame->s3);
    dbgprint("s4: %p\n", frame->s4);
    dbgprint("s5: %p\t", frame->s5);
    dbgprint("s6: %p\t", frame->s6);
    dbgprint("s7: %p\t", frame->s7);
    dbgprint("s8: %p\n", frame->s8);
    dbgprint("s9: %p\t", frame->s9);
    dbgprint("s10: %p\t", frame->s10);
    dbgprint("s11: %p\t", frame->s11);
    dbgprint("ra: %p\n", frame->ra);
    dbgprint("sp: %p\t", frame->sp);
    dbgprint("gp: %p\t", frame->gp);
    dbgprint("tp: %p\t", frame->tp);
    dbgprint("epc: %p\n", frame->epc);
}

// Check if it's an external/software interrupt, 
// and handle it. 
void interrupt_dispatch(trap_frame_t *frame) 
{
	uint64_t scause = r_scause();   // 获取中断产生的原因

	#ifdef QEMU 
	// handle external interrupt 
	if ((SCAUSE_INTERRUPT & scause) && SCAUSE_S_EXTERNAL_INTR == (scause & 0xff)) 
	#else 
	// on k210, supervisor software interrupt is used 
	// in alternative to supervisor external interrupt, 
	// which is not available on k210. 
	if ((SCAUSE_INTERRUPT | SCAUSE_S_SOFTWARE_INTR) == scause && SCAUSE_S_EXTERNAL_INTR == r_stval()) 
	#endif 
    {
        // keprintln("external interrupt");
        interrupt_do_irq(frame);
        softirq_handle_in_interrupt();
    } else if ((SCAUSE_INTERRUPT | SCAUSE_S_TIMER_INTR) == scause) {
        //keprintln("timer interrupt");
        /* 直接调用内核的时钟处理 */
        clock_handler(-1, NULL);
        //clock_handler2(-1, NULL);
        /* 处理完后需要设置下一个超时的时钟时间，才能再次产生中断 */
        timer_interrupt_set_next_timeout();
        /* 调用软中断 */
        softirq_handle_in_interrupt();
    } else if (SCAUSE_INTERRUPT & scause) {
        errprintln("unsupported other external interrupt!");
        errprintln("\nscause %p\n", scause);
        errprintln("sepc=%p stval=%p hart=%d\n", r_sepc(), r_stval(), r_tp());
        panic("interrupt_dispatch");
    } else {
        /* 内核异常处理: cause 异常值是 [0-15] */
        int exception = scause & 0xff;
        interrupt_handler_t handler = interrupt_handlers[exception];
        if (handler) {
            handler(frame);
        } else {
            keprint("\nscause %p\n", scause);
            keprint("sepc=%p stval=%p hart=%d\n", r_sepc(), r_stval(), r_tp());
            panic("[interrupt] %d handler null", exception);        
        }
    }
}

void exception_frame_build(uint32_t code, exception_handler_t handler, trap_frame_t *frame)
{
    noteprint("exception_frame_build: empty function!");
}

int exception_return(trap_frame_t *frame)
{
    noteprint("exception_frame_build: empty function!");
    return -1;
}