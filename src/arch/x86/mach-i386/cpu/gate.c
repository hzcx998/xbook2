#include <arch/segment.h>
#include <arch/gate.h>
#include <arch/interrupt.h>
#include <arch/pic.h>
#include <arch/registers.h>
#include <xbook/kernel.h>

/* 
 * Interrupt descriptor table
 */
struct gate_descriptor *idt0;

#define KERN_SYSCALL_NR 0x40

static void gate_descriptor_set(struct gate_descriptor *descriptor, interrupt_handler_t offset,
		unsigned int selector, unsigned int attributes, unsigned char privilege)
{
	descriptor->offset_low   = (unsigned int)offset & 0xffff;
	descriptor->selector    = selector;
	descriptor->attributes	= attributes | (privilege << 5);
	descriptor->datacount   = 0;
	descriptor->offset_high  = ((unsigned int)offset >> 16) & 0xffff;
}

static void interrupt_descriptor_init()
{
	idt0 = (struct gate_descriptor *) IDT_VADDR;
	
	/*
	 将中断描述符表的内容设置成内核下的中断门
	 并把汇编部分的中断处理函数传入进去
	 */
	int i;
   	for (i = 0; i < MAX_IDT_NR; i++) {
      	gate_descriptor_set(IDT_OFF2PTR(idt0, i), 0, 0, 0, 0); 
   	}
	gate_descriptor_set(IDT_OFF2PTR(idt0, 0x00), exception_entry0x00, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	gate_descriptor_set(IDT_OFF2PTR(idt0, 0x01), exception_entry0x01, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	gate_descriptor_set(IDT_OFF2PTR(idt0, 0x02), exception_entry0x02, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	gate_descriptor_set(IDT_OFF2PTR(idt0, 0x03), exception_entry0x03, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	gate_descriptor_set(IDT_OFF2PTR(idt0, 0x04), exception_entry0x04, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	gate_descriptor_set(IDT_OFF2PTR(idt0, 0x05), exception_entry0x05, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	gate_descriptor_set(IDT_OFF2PTR(idt0, 0x06), exception_entry0x06, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	gate_descriptor_set(IDT_OFF2PTR(idt0, 0x07), exception_entry0x07, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	gate_descriptor_set(IDT_OFF2PTR(idt0, 0x08), exception_entry0x08, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	gate_descriptor_set(IDT_OFF2PTR(idt0, 0x09), exception_entry0x09, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	gate_descriptor_set(IDT_OFF2PTR(idt0, 0x0a), exception_entry0x0a, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	gate_descriptor_set(IDT_OFF2PTR(idt0, 0x0b), exception_entry0x0b, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	gate_descriptor_set(IDT_OFF2PTR(idt0, 0x0c), exception_entry0x0c, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	gate_descriptor_set(IDT_OFF2PTR(idt0, 0x0d), exception_entry0x0d, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	gate_descriptor_set(IDT_OFF2PTR(idt0, 0x0e), exception_entry0x0e, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	gate_descriptor_set(IDT_OFF2PTR(idt0, 0x0f), exception_entry0x0f, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	gate_descriptor_set(IDT_OFF2PTR(idt0, 0x10), exception_entry0x10, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	gate_descriptor_set(IDT_OFF2PTR(idt0, 0x11), exception_entry0x11, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	gate_descriptor_set(IDT_OFF2PTR(idt0, 0x12), exception_entry0x12, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	gate_descriptor_set(IDT_OFF2PTR(idt0, 0x13), exception_entry0x13, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	gate_descriptor_set(IDT_OFF2PTR(idt0, 0x14), exception_entry0x14, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	gate_descriptor_set(IDT_OFF2PTR(idt0, 0x15), exception_entry0x15, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	gate_descriptor_set(IDT_OFF2PTR(idt0, 0x16), exception_entry0x16, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	gate_descriptor_set(IDT_OFF2PTR(idt0, 0x17), exception_entry0x17, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	gate_descriptor_set(IDT_OFF2PTR(idt0, 0x18), exception_entry0x18, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	gate_descriptor_set(IDT_OFF2PTR(idt0, 0x19), exception_entry0x19, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	gate_descriptor_set(IDT_OFF2PTR(idt0, 0x1a), exception_entry0x1a, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	gate_descriptor_set(IDT_OFF2PTR(idt0, 0x1b), exception_entry0x1b, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	gate_descriptor_set(IDT_OFF2PTR(idt0, 0x1c), exception_entry0x1c, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	gate_descriptor_set(IDT_OFF2PTR(idt0, 0x1d), exception_entry0x1d, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	gate_descriptor_set(IDT_OFF2PTR(idt0, 0x1e), exception_entry0x1e, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	gate_descriptor_set(IDT_OFF2PTR(idt0, 0x1f), exception_entry0x1f, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	
	gate_descriptor_set(IDT_OFF2PTR(idt0, 0x20), irq_entry0x20, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	gate_descriptor_set(IDT_OFF2PTR(idt0, 0x21), irq_entry0x21, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	gate_descriptor_set(IDT_OFF2PTR(idt0, 0x22), irq_entry0x22, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	gate_descriptor_set(IDT_OFF2PTR(idt0, 0x23), irq_entry0x23, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	gate_descriptor_set(IDT_OFF2PTR(idt0, 0x24), irq_entry0x24, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	gate_descriptor_set(IDT_OFF2PTR(idt0, 0x25), irq_entry0x25, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	gate_descriptor_set(IDT_OFF2PTR(idt0, 0x26), irq_entry0x26, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	gate_descriptor_set(IDT_OFF2PTR(idt0, 0x27), irq_entry0x27, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	gate_descriptor_set(IDT_OFF2PTR(idt0, 0x28), irq_entry0x28, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	gate_descriptor_set(IDT_OFF2PTR(idt0, 0x29), irq_entry0x29, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	gate_descriptor_set(IDT_OFF2PTR(idt0, 0x2a), irq_entry0x2a, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	gate_descriptor_set(IDT_OFF2PTR(idt0, 0x2b), irq_entry0x2b, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	gate_descriptor_set(IDT_OFF2PTR(idt0, 0x2c), irq_entry0x2c, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	gate_descriptor_set(IDT_OFF2PTR(idt0, 0x2d), irq_entry0x2d, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	gate_descriptor_set(IDT_OFF2PTR(idt0, 0x2e), irq_entry0x2e, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	gate_descriptor_set(IDT_OFF2PTR(idt0, 0x2f), irq_entry0x2f, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL0); 
	
	/* 系统调用处理中断 */
	gate_descriptor_set(IDT_OFF2PTR(idt0, KERN_SYSCALL_NR), syscall_handler, KERNEL_CODE_SEL, DA_386IGate, DA_GATE_DPL3);

	idt_register_set(IDT_LIMIT, IDT_VADDR);
}

void gate_descriptor_init()
{
	interrupt_descriptor_init();
    interrupt_expection_init();
}
