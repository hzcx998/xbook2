#ifndef _X86_GATE_H
#define _X86_GATE_H

#define IDT_VADDR		(KERN_BASE_VIR_ADDR + 0x003F0800)  /* IDT 的虚拟地址 */
#define IDT_LIMIT		0x000007ff

#define MAX_IDT_NR (IDT_LIMIT/8)      

#define IDT_OFF2PTR(idt, off)    (idt + off) 

#define	DA_TaskGate		0x85	/* 任务门类型值				*/
#define	DA_386CGate		0x8C	/* 386 调用门类型值			*/
#define	DA_386IGate		0x8E	/* 386 中断门类型值			*/
#define	DA_386TGate		0x8F	/* 386 陷阱门类型值			*/

#define DA_GATE_DPL0 0
#define DA_GATE_DPL1 1
#define DA_GATE_DPL2 2
#define DA_GATE_DPL3 3

struct gate_descriptor {
	unsigned short offset_low, selector;
	unsigned char datacount;
	unsigned char attributes;		/* P(1) DPL(2) DT(1) TYPE(4) */
	unsigned short offset_high;
};

void gate_descriptor_init();


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

extern void syscall_handler();

#endif	/* _X86_GATE_H */