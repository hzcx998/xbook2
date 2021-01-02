#include <arch/interrupt.h>
#include <arch/registers.h>
#include <xbook/debug.h>
#include <xbook/exception.h>
#include <xbook/schedule.h>
#include <xbook/exception.h>
#include <xbook/syscall.h>
#include <arch/segment.h>
#include <string.h>

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
    if (task_init_done) {
        switch (frame->vec_no) {
        case EP_PAGE_FAULT:
            page_do_fault(frame);
            break;
        case EP_DIVIDE:
        case EP_INVALID_OPCODE:
            exception_force_self(EXP_CODE_ILL);
            break;
        case EP_DEVICE_NOT_AVAILABLE:
            exception_force_self(EXP_CODE_DEVICE);
            break;
        case EP_COPROCESSOR_SEGMENT_OVERRUN:
        case EP_X87_FLOAT_POINT:
        case EP_SIMD_FLOAT_POINT:
            exception_force_self(EXP_CODE_FPE);
            break;
        case EP_OVERFLOW:
        case EP_BOUND_RANGE:
        case EP_INVALID_TSS:
        case EP_ALIGNMENT_CHECK:
            exception_force_self(EXP_CODE_BUS);
            break;
        case EP_SEGMENT_NOT_PRESENT:
        case EP_GENERAL_PROTECTION:
            exception_force_self(EXP_CODE_SEGV);
            break;
        case EP_STACK_FAULT:
            exception_force_self(EXP_CODE_STKFLT);
            break;
        case EP_MACHINE_CHECK:
        case EP_INTERRUPT:
            exception_force_self(EXP_CODE_INT);
            break;
        case EP_DOUBLE_FAULT:
            exception_force_self(EXP_CODE_FINALHIT);
            break;
        case EP_DEBUG:
        case EP_BREAKPOINT:
            exception_force_self(EXP_CODE_TRAP);
            break;
        default:
            break;
        }
    } else {
        switch (frame->vec_no) {
        case EP_PAGE_FAULT:
            {
                unsigned long addr = 0x00;
                addr = cpu_cr2_read(); /* cr2 saved the fault addr */
                keprint("page fault addr:%x\n", addr);
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
        case EP_DEBUG:
        case EP_BREAKPOINT:
        default:
            break;
        }
        trap_frame_dump(frame);
        panic("Expection not resuloved!\n");
    }
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
    keprint(PRINT_DEBUG "Trap frame:\n");
    keprint("Vector name: %s \n", interrupt_names[frame->vec_no]);

    keprint(PRINT_DEBUG "Vector:%d edi:%x esi:%x ebp:%x esp dummy:%x ebx:%x edx:%x ecx:%x eax:%x\n",
        frame->vec_no, frame->edi, frame->esi, frame->ebp, frame->esp_dummy,
        frame->ebx, frame->edx, frame->ecx, frame->eax);
    keprint(PRINT_DEBUG "gs:%x fs:%x es:%x ds:%x error code:%x eip:%x cs:%x eflags:%x esp:%x ss:%x\n",
        frame->gs, frame->fs, frame->es, frame->ds, frame->error_code,
        frame->eip, frame->cs, frame->eflags, frame->esp, frame->ss);
	if(frame->error_code != 0xFFFFFFFF){
		if(frame->error_code & 1){
			keprint(PRINT_DEBUG "    External Event: NMI,hard interruption,ect.\n");
		}else{
			keprint(PRINT_DEBUG "    Not External Event: inside.\n");
		}
		if(frame->error_code & (1 << 1)){
			keprint(PRINT_DEBUG "    IDT: selector in idt.\n");
		}else{
			keprint(PRINT_DEBUG "    IDT: selector in gdt or ldt.\n");
		}
		if(frame->error_code & (1 <<2 )){
			keprint(PRINT_DEBUG "    TI: selector in ldt.\n");
		}else{
			keprint(PRINT_DEBUG "    TI: selector in gdt.\n");
		}
		keprint(PRINT_DEBUG "    Selector: idx %d\n", (frame->error_code&0xfff8)>>3);
	}
}

void exception_frame_build(uint32_t code, exception_handler_t handler, trap_frame_t *frame)
{
    unsigned long flags;
    interrupt_save_and_disable(flags);
    exception_frame_t *exp_frame = (exception_frame_t *)((frame->esp - sizeof(exception_frame_t)) & -8UL);
    exp_frame->code = code;
    memcpy(&exp_frame->trap_frame, frame, sizeof(trap_frame_t));
    exp_frame->ret_addr = exp_frame->ret_code;

    /* 构建返回代码，系统调用封装,模拟系统调用来实现从用户态返回到内核态
    mov eax, SYS_EXPRET
    int 0x40 */
    exp_frame->ret_code[0] = 0xb8;
    *(uint32_t *)(exp_frame->ret_code + 1) = SYS_EXPRET;    /* 把系统调用号填进去 */
    *(uint16_t *)(exp_frame->ret_code + 5) = 0x40cd;        /* int对应的指令是0xcd，系统调用中断号是0x40 */
    
    frame->eip = (unsigned int)handler;
    frame->esp = (unsigned int)exp_frame;
    frame->ds = frame->es = frame->fs = frame->gs = USER_DATA_SEL;
    frame->ss = USER_STACK_SEL;
    frame->cs = USER_CODE_SEL;
    interrupt_restore_state(flags);
}

int exception_return(trap_frame_t *frame)
{
    exception_frame_t *exp_frame = (exception_frame_t *)(frame->esp - 4);
    exception_manager_t *exception_manager = &task_current->exception_manager; 
    unsigned long irq_flags;
    spin_lock_irqsave(&exception_manager->manager_lock, irq_flags);
    exception_manager->in_user_mode = 0;
    memcpy(frame, &exp_frame->trap_frame, sizeof(trap_frame_t));
    spin_unlock_irqrestore(&exception_manager->manager_lock, irq_flags);
    return frame->eax;
}