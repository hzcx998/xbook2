#ifndef _X86_GATE_H
#define _X86_GATE_H

/* IDT 的虚拟地址 */
#define IDT_VADDR		0x80200800
#define IDT_LIMIT		0x000007ff

#define	DA_TaskGate		0x85	/* 任务门类型值				*/
#define	DA_386CGate		0x8C	/* 386 调用门类型值			*/
#define	DA_386IGate		0x8E	/* 386 中断门类型值			*/
#define	DA_386TGate		0x8F	/* 386 陷阱门类型值			*/

#define DA_GATE_DPL0 0
#define DA_GATE_DPL1 1
#define DA_GATE_DPL2 2
#define DA_GATE_DPL3 3

/*
门描述符结构
*/
struct gate_descriptor {
	unsigned short offset_low, selector;
	unsigned char datacount;
	unsigned char attributes;		/* P(1) DPL(2) DT(1) TYPE(4) */
	unsigned short offset_high;
};

void init_gate_descriptor();

#endif	/* _X86_GATE_H */