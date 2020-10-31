#include <arch/interrupt.h>
#include <arch/registers.h>
#include <xbook/debug.h>
#include <xbook/trigger.h>
#include <xbook/schedule.h>

#define DEBUG_INTR

interrupt_handler_t interrupt_handlers[MAX_INTERRUPT_NR];
char* interrupt_names[MAX_INTERRUPT_NR];		     

void interrupt_general_handler(unsigned int esp) 
{
	trap_frame_t *frame = (trap_frame_t *)((unsigned int )esp);

    //IRQ7和IRQ15可能会产生伪中断(spurious interrupt),无须处理。
	if (frame->vec_no == 0x27 || frame->vec_no == 0x2f) {	
      	return;		
   	}
    switch (frame->vec_no) {
    case EP_PAGE_FAULT:
        if (task_init_done) {
            page_do_fault(frame);
    		return;
        } else {
            unsigned long addr = 0x00;
            addr = cpu_cr2_read(); /* cr2 saved the fault addr */
            printk("page fault addr:%x\n", addr);
        }
    case EP_DIVIDE:
    case EP_DEVICE_NOT_AVAILABLE:
    case EP_COPROCESSOR_SEGMENT_OVERRUN:
    case EP_X87_FLOAT_POINT:
    case EP_SIMD_FLOAT_POINT:
    case EP_OVERFLOW:
	case EP_BOUND_RANGE:
    case EP_SEGMENT_NOT_PRESENT:
    case EP_STACK_FAULT:
	case EP_INVALID_TSS:
    case EP_GENERAL_PROTECTION:
    case EP_ALIGNMENT_CHECK:
    case EP_MACHINE_CHECK:
    case EP_INVALID_OPCODE:
	case EP_INTERRUPT:
    case EP_DOUBLE_FAULT:
        if (task_init_done) {
            #ifdef DEBUG_INTR
            printk(KERN_EMERG "interrupt_general_handler: touch TRIGHW trigger because a expection %d occur!\n", frame->vec_no);
            #endif
            trigger_force(TRIGSYS, current_task->pid);
            trap_frame_dump(frame);
		    return;
        }
	case EP_DEBUG:
    case EP_BREAKPOINT:
        if (task_init_done) {
            #ifdef DEBUG_INTR
		    printk(KERN_NOTICE "interrupt_general_handler: touch TRIGDBG trigger because a debug occur!\n");
            #endif
            trigger_force(TRIGDBG, current_task->pid);
            trap_frame_dump(frame);
            return;
        }
    default:
		break;
	}
    trap_frame_dump(frame);
   	panic("Expection not resuloved!\n");
}

void interrupt_expection_init(void)
{
	int i;
   	for (i = 0; i < MAX_INTERRUPT_NR; i++) {
      	interrupt_handlers[i] = interrupt_general_handler;		    
      	interrupt_names[i] = "unknown";    
   	}
	interrupt_names[0] = "#DE Divide Error";
	interrupt_names[1] = "#DB Debug Exception";
	interrupt_names[2] = "NMI Interrupt";
	interrupt_names[3] = "#BP Breakpoint Exception";
	interrupt_names[4] = "#OF Overflow Exception";
	interrupt_names[5] = "#BR BOUND Range Exceeded Exception";
	interrupt_names[6] = "#UD Invalid Opcode Exception";
	interrupt_names[7] = "#NM Device Not Available Exception";
	interrupt_names[8] = "#DF Double Fault Exception";
	interrupt_names[9] = "Coprocessor Segment Overrun";
	interrupt_names[10] = "#TS Invalid TSS Exception";
	interrupt_names[11] = "#NP Segment Not Present";
	interrupt_names[12] = "#SS Stack Fault Exception";
	interrupt_names[13] = "#GP General Protection Exception";
	interrupt_names[14] = "#PF Page-Fault Exception";
	interrupt_names[15] = "Reserved"; //第15项是intel保留项，未使用
	interrupt_names[16] = "#MF x87 FPU Floating-Point Error";
	interrupt_names[17] = "#AC Alignment Check Exception";
	interrupt_names[18] = "#MC Machine-Check Exception";
	interrupt_names[19] = "#XF SIMD Floating-Point Exception";
}

void interrupt_register_handler(unsigned char interrupt, interrupt_handler_t function) 
{
   	interrupt_handlers[interrupt] = function; 
}

void unregister_interrupt_handler(unsigned char interrupt)
{
   	interrupt_handlers[interrupt] = interrupt_general_handler; 
}

int irq_register_handler(unsigned char irq, interrupt_handler_t function) 
{
	if (irq < IRQ0 || irq > IRQ15) {
		return -1;
	}
   	interrupt_handlers[IRQ_OFF_IN_IDT + irq] = function; 
    return 0;
}

int irq_unregister_handler(unsigned char irq)
{
	if (irq < IRQ0_CLOCK || irq > IRQ15_RESERVE) {
		return -1;
	}
	interrupt_handlers[IRQ_OFF_IN_IDT + irq] = interrupt_general_handler; 
    return 0;
}

void trap_frame_dump(trap_frame_t *frame)
{
    printk(KERN_DEBUG "Trap frame:\n");
    printk("Vector name: %s \n", interrupt_names[frame->vec_no]);

    printk(KERN_DEBUG "Vector:%d edi:%x esi:%x ebp:%x esp dummy:%x ebx:%x edx:%x ecx:%x eax:%x\n",
        frame->vec_no, frame->edi, frame->esi, frame->ebp, frame->esp_dummy,
        frame->ebx, frame->edx, frame->ecx, frame->eax);
    printk(KERN_DEBUG "gs:%x fs:%x es:%x ds:%x error code:%x eip:%x cs:%x eflags:%x esp:%x ss:%x\n",
        frame->gs, frame->fs, frame->es, frame->ds, frame->error_code,
        frame->eip, frame->cs, frame->eflags, frame->esp, frame->ss);
	if(frame->error_code != 0xFFFFFFFF){
		if(frame->error_code & 1){
			printk(KERN_DEBUG "    External Event: NMI,hard interruption,ect.\n");
		}else{
			printk(KERN_DEBUG "    Not External Event: inside.\n");
		}
		if(frame->error_code & (1 << 1)){
			printk(KERN_DEBUG "    IDT: selector in idt.\n");
		}else{
			printk(KERN_DEBUG "    IDT: selector in gdt or ldt.\n");
		}
		if(frame->error_code & (1 <<2 )){
			printk(KERN_DEBUG "    TI: selector in ldt.\n");
		}else{
			printk(KERN_DEBUG "    TI: selector in gdt.\n");
		}
		printk(KERN_DEBUG "    Selector: idx %d\n", (frame->error_code&0xfff8)>>3);
	}
}
