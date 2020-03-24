#include "interrupt.h"
#include "registers.h"
#include <xbook/debug.h>

// 中断处理函数组成的数组
intr_handler_t intr_handler_table[MAX_INTERRUPT_NR];

// 用于保存异常的名字
char* intr_name_table[MAX_INTERRUPT_NR];		     

/* 
 * 通用的中断处理函数,一般用在异常出现时的处理 
 */
void intr_general_handler(unsigned int esp) 
{
	// 中断栈
	trap_frame_t *frame = (trap_frame_t *)((unsigned int )esp);

	// 0x2f是从片8259A上的最后一个irq引脚，保留
	if (frame->vec_no == 0x27 || frame->vec_no == 0x2f) {	
      	return;		//IRQ7和IRQ15会产生伪中断(spurious interrupt),无须处理。
   	}
	#if 0
    /* 支持信号后的处理 */
    switch (frame->vec_no) {
    case EP_DIVIDE:
    case EP_DEVICE_NOT_AVAILABLE:
    case EP_COPROCESSOR_SEGMENT_OVERRUN:
    case EP_X87_FLOAT_POINT:
    case EP_SIMD_FLOAT_POINT:

        //ForceSignal(SIGFPE, SysGetPid());
        return;
    case EP_OVERFLOW:
        //ForceSignal(SIGSTKFLT, SysGetPid());
        return;
    case EP_BOUND_RANGE:
    case EP_SEGMENT_NOT_PRESENT:
    case EP_STACK_FAULT:

        //ForceSignal(SIGSEGV, SysGetPid());
        return;
    case EP_INVALID_TSS:
    case EP_GENERAL_PROTECTION:
    case EP_ALIGNMENT_CHECK:
    case EP_MACHINE_CHECK:
        //ForceSignal(SIGBUS, SysGetPid());
        return;
    case EP_INVALID_OPCODE:

        //ForceSignal(SIGILL, SysGetPid());
        return;
    case EP_DEBUG:
    case EP_BREAKPOINT:

        //ForceSignal(SIGTRAP, SysGetPid());
        return;
    case EP_PAGE_FAULT:
        /* 后面处理 */
        break;
    case EP_INTERRUPT:
    case EP_DOUBLE_FAULT:

        //ForceSignal(SIGABRT, SysGetPid());
        return;
    }
    #endif
    /* 原始处理方式 */

	printk("! Exception messag start.\n");
	printk("name: %s \n", intr_name_table[frame->vec_no]);

	printk("vec: %x\n", 
			frame->vec_no);
	//Panic("expection");
	printk("edi: %x esi: %x ebp: %x esp: %x\n", 
			frame->edi, frame->esi, frame->ebp, frame->esp);
	printk("ebx: %x edx: %x ecx: %x eax: %x\n", 
			frame->ebx, frame->edx, frame->ecx, frame->eax);
	printk("gs: %x fs: %x es: %x ds: %x\n", 
			frame->gs, frame->fs, frame->es, frame->ds);
	printk("err: %x eip: %x cs: %x eflags: %x\n", 
			frame->error_code, frame->eip, frame->cs, frame->eflags);
	printk("esp: %x ss: %x\n", 
			frame->esp, frame->ss);
	
	if(frame->error_code != 0xFFFFFFFF){
		printk("Error code:%x\n", frame->error_code);
		
		if(frame->error_code & 1){
			printk("    External Event: NMI,hard interruption,ect.\n");
		}else{
			printk("    Not External Event: inside.\n");
		}
		if(frame->error_code & (1 << 1)){
			printk("    IDT: selector in idt.\n");
		}else{
			printk("    IDT: selector in gdt or ldt.\n");
		}
		if(frame->error_code & (1 <<2 )){
			printk("    TI: selector in ldt.\n");
		}else{
			printk("    TI: selector in gdt.\n");
		}
		printk("    Selector: idx %d\n", (frame->error_code&0xfff8)>>3);
	}

    if (frame->vec_no == EP_PAGE_FAULT) {
        unsigned int pageFaultVaddr = 0; 
		pageFaultVaddr = read_cr2();
		printk("page fault addr is: %x\n", pageFaultVaddr);
    }
	/*printk("    task %s %x kstack %x.\n", task->name, task, task->kstack);
	printk("    pgdir %x cr3 %x\n", task->pgdir, PageAddrV2P((unsigned int)task->pgdir));
	*/	
	printk("! Exception Messag done.\n");
  	// 能进入中断处理程序就表示已经处在关中断情况下,
  	// 不会出现调度进程的情况。故下面的死循环不会再被中断。
   	panic("expection");
}

/* 完成一般中断处理函数注册及异常名称注册 */
void init_intr_expection(void)
{
	int i;
	//设置中断处理函数
   	for (i = 0; i < MAX_INTERRUPT_NR; i++) {

		// 默认为intr_general_handler
      	intr_handler_table[i] = intr_general_handler;		    
		
		// 以后会由RegisterHandler来注册具体处理函数。

		// 先统一赋值为unknown 
      	intr_name_table[i] = "unknown";    
   	}
	intr_name_table[0] = "#DE Divide Error";
	intr_name_table[1] = "#DB Debug Exception";
	intr_name_table[2] = "NMI Interrupt";
	intr_name_table[3] = "#BP Breakpoint Exception";
	intr_name_table[4] = "#OF Overflow Exception";
	intr_name_table[5] = "#BR BOUND Range Exceeded Exception";
	intr_name_table[6] = "#UD Invalid Opcode Exception";
	intr_name_table[7] = "#NM Device Not Available Exception";
	intr_name_table[8] = "#DF Double Fault Exception";
	intr_name_table[9] = "Coprocessor Segment Overrun";
	intr_name_table[10] = "#TS Invalid TSS Exception";
	intr_name_table[11] = "#NP Segment Not Present";
	intr_name_table[12] = "#SS Stack Fault Exception";
	intr_name_table[13] = "#GP General Protection Exception";
	intr_name_table[14] = "#PF Page-Fault Exception";
	intr_name_table[15] = "Reserved"; //第15项是intel保留项，未使用
	intr_name_table[16] = "#MF x87 FPU Floating-Point Error";
	intr_name_table[17] = "#AC Alignment Check Exception";
	intr_name_table[18] = "#MC Machine-Check Exception";
	intr_name_table[19] = "#XF SIMD Floating-Point Exception";
}

/* 
 * 在中断处理程序数组第vector_no个元素中注册安装中断处理程序function
 */
void __register_intr_handler(unsigned char interrupt, intr_handler_t function) 
{
	//把函数写入到中断处理程序表
   	intr_handler_table[interrupt] = function; 
}

void __unregister_intr_handler(unsigned char interrupt)
{
	//把默认函数写入到中断处理程序表
   	intr_handler_table[interrupt] = intr_general_handler; 
}

/* 
 * 注册IRQ中断
 */
void __register_irq_handler(unsigned char irq, intr_handler_t function) 
{
	/* 如果是不正确的irq号就退出 */
	if (irq < IRQ0_CLOCK || irq > IRQ15_RESERVE) {
		return;
	}
	//把函数写入到中断处理程序表
   	intr_handler_table[IRQ_START + irq] = function; 
}

/* 
 * 取消IRQ中断
 */
void __unregister_irq_handler(unsigned char irq)
{
	/* 如果是不正确的irq号就退出 */
	if (irq < IRQ0_CLOCK || irq > IRQ15_RESERVE) {
		return;
	}
	//把默认函数写入到中断处理程序表
   	intr_handler_table[IRQ_START + irq] = intr_general_handler; 
}

void dump_trap_frame(trap_frame_t *frame)
{
#ifdef _DEBUG_GATE
    printk(PART_TIP "----Trap Frame----\n");
    printk(PART_TIP "vector:%d edi:%x esi:%x ebp:%x esp dummy:%x ebx:%x edx:%x ecx:%x eax:%x\n",
        frame->edi, frame->esi, frame->ebp, frame->espDummy, frame->ebx, frame->edx, frame->ecx, frame->eax);
    printk(PART_TIP "gs:%x fs:%x es:%x ds:%x error code:%x eip:%x cs:%x eflags:%x esp:%x ss:%x\n",
        frame->gs, frame->fs, frame->es, frame->ds, frame->error_code, frame->eip, frame->cs, frame->eflags, frame->esp, frame->ss);
#endif 
}
