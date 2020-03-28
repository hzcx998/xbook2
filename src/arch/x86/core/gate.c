#include "segment.h"
#include "gate.h"
#include "interrupt.h"
#include "pic.h"
#include <xbook/kernel.h>

/* 
 * Interrupt descriptor table
 */
struct gate_descriptor *idt;

// 总共支持的中断数
#define MAX_IDT_NR (IDT_LIMIT/8)      

/* 用户消息int软中断号，用于从用户态切换到内核态执行操作 */
#define KERN_USRMSG_NR 0x40

extern void exception_entry0x00();
extern void exception_entry0x01();
extern void exception_entry0x02();
extern void exception_entry0x03();
extern void exception_entry0x04();
extern void exception_entry0x05();
extern void exception_entry0x06();
extern void exception_entry0x07();
extern void exception_entry0x08();
extern void exception_entry0x09();
extern void exception_entry0x0a();
extern void exception_entry0x0b();
extern void exception_entry0x0c();
extern void exception_entry0x0d();
extern void exception_entry0x0e();
extern void exception_entry0x0f();
extern void exception_entry0x10();
extern void exception_entry0x11();
extern void exception_entry0x12();
extern void exception_entry0x13();
extern void exception_entry0x14();
extern void exception_entry0x15();
extern void exception_entry0x16();
extern void exception_entry0x17();
extern void exception_entry0x18();
extern void exception_entry0x19();
extern void exception_entry0x1a();
extern void exception_entry0x1b();
extern void exception_entry0x1c();
extern void exception_entry0x1d();
extern void exception_entry0x1e();
extern void exception_entry0x1f();

extern void irq_entry0x20();
extern void irq_entry0x21();
extern void irq_entry0x22();
extern void irq_entry0x23();
extern void irq_entry0x24();
extern void irq_entry0x25();
extern void irq_entry0x26();
extern void irq_entry0x27();
extern void irq_entry0x28();
extern void irq_entry0x29();
extern void irq_entry0x2a();
extern void irq_entry0x2b();
extern void irq_entry0x2c();
extern void irq_entry0x2d();
extern void irq_entry0x2e();
extern void irq_entry0x2f();

extern void kern_usrmsg_handler();

static void set_gate_descriptor(struct gate_descriptor *descriptor, intr_handler_t offset,
		unsigned int selector, unsigned int attributes, unsigned char privilege)
{
	descriptor->offset_low   = (unsigned int)offset & 0xffff;
	descriptor->selector    = selector;
	descriptor->attributes	= attributes | (privilege << 5);
	descriptor->datacount   = 0;
	descriptor->offset_high  = ((unsigned int)offset >> 16) & 0xffff;

}

static void init_interrupt_descriptor()
{
	idt = (struct gate_descriptor *) IDT_VADDR;
	
	/*
	 将中断描述符表的内容设置成内核下的中断门
	 并把汇编部分的中断处理函数传入进去
	 */
	int i;
   	for (i = 0; i < MAX_IDT_NR; i++) {
      	set_gate_descriptor(&idt[i], 0, 0, 0, 0); 
   	}
	/*
	 异常的中断入口
	 */
	set_gate_descriptor(&idt[0x00], exception_entry0x00, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	set_gate_descriptor(&idt[0x01], exception_entry0x01, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	set_gate_descriptor(&idt[0x02], exception_entry0x02, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	set_gate_descriptor(&idt[0x03], exception_entry0x03, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	set_gate_descriptor(&idt[0x04], exception_entry0x04, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	set_gate_descriptor(&idt[0x05], exception_entry0x05, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	set_gate_descriptor(&idt[0x06], exception_entry0x06, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	set_gate_descriptor(&idt[0x07], exception_entry0x07, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	set_gate_descriptor(&idt[0x08], exception_entry0x08, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	set_gate_descriptor(&idt[0x09], exception_entry0x09, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	set_gate_descriptor(&idt[0x0a], exception_entry0x0a, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	set_gate_descriptor(&idt[0x0b], exception_entry0x0b, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	set_gate_descriptor(&idt[0x0c], exception_entry0x0c, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	set_gate_descriptor(&idt[0x0d], exception_entry0x0d, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	set_gate_descriptor(&idt[0x0e], exception_entry0x0e, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	set_gate_descriptor(&idt[0x0f], exception_entry0x0f, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	set_gate_descriptor(&idt[0x10], exception_entry0x10, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	set_gate_descriptor(&idt[0x11], exception_entry0x11, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	set_gate_descriptor(&idt[0x12], exception_entry0x12, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	set_gate_descriptor(&idt[0x13], exception_entry0x13, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	set_gate_descriptor(&idt[0x14], exception_entry0x14, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	set_gate_descriptor(&idt[0x15], exception_entry0x15, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	set_gate_descriptor(&idt[0x16], exception_entry0x16, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	set_gate_descriptor(&idt[0x17], exception_entry0x17, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	set_gate_descriptor(&idt[0x18], exception_entry0x18, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	set_gate_descriptor(&idt[0x19], exception_entry0x19, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	set_gate_descriptor(&idt[0x1a], exception_entry0x1a, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	set_gate_descriptor(&idt[0x1b], exception_entry0x1b, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	set_gate_descriptor(&idt[0x1c], exception_entry0x1c, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	set_gate_descriptor(&idt[0x1d], exception_entry0x1d, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	set_gate_descriptor(&idt[0x1e], exception_entry0x1e, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	set_gate_descriptor(&idt[0x1f], exception_entry0x1f, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	
	/*
	 IRQ的中断入口
	 */
	set_gate_descriptor(&idt[0x20], irq_entry0x20, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	set_gate_descriptor(&idt[0x21], irq_entry0x21, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	set_gate_descriptor(&idt[0x22], irq_entry0x22, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	set_gate_descriptor(&idt[0x23], irq_entry0x23, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	set_gate_descriptor(&idt[0x24], irq_entry0x24, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	set_gate_descriptor(&idt[0x25], irq_entry0x25, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	set_gate_descriptor(&idt[0x26], irq_entry0x26, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	set_gate_descriptor(&idt[0x27], irq_entry0x27, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	set_gate_descriptor(&idt[0x28], irq_entry0x28, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	set_gate_descriptor(&idt[0x29], irq_entry0x29, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	set_gate_descriptor(&idt[0x2a], irq_entry0x2a, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	set_gate_descriptor(&idt[0x2b], irq_entry0x2b, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	set_gate_descriptor(&idt[0x2c], irq_entry0x2c, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	set_gate_descriptor(&idt[0x2d], irq_entry0x2d, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	set_gate_descriptor(&idt[0x2e], irq_entry0x2e, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	set_gate_descriptor(&idt[0x2f], irq_entry0x2f, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	
	/* 内核态用户消息处理中断 */
	set_gate_descriptor(&idt[KERN_USRMSG_NR], kern_usrmsg_handler, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL3);

	load_idtr(IDT_LIMIT, IDT_VADDR);

}

void init_gate_descriptor()
{
	/* interrupt gate */
	init_interrupt_descriptor();
	
    /* intr expection setting */
    init_intr_expection();

    /* init pic controller */
    init_pic();
    
}
