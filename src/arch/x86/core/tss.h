#ifndef _X86_TSS_H
#define _X86_TSS_H

#include <xbook/stdint.h>
#include <xbook/task.h>

/* 内核栈 */
#define KERNEL_STATCK_TOP		0x8009f000
#define KERNEL_STATCK_BOTTOM	(KERNEL_STATCK_TOP - (TASK_KSTACK_SIZE))

/* 任务状态 */
typedef struct tss 
{
	uint32_t backlink;
	uint32_t esp0;	//we will use esp 
	uint32_t ss0;	//stack segment
	uint32_t esp1;
	uint32_t ss1;
	uint32_t esp2;
	uint32_t ss2;
	uint32_t cr3;
	uint32_t eip;
	uint32_t eflags;
	uint32_t eax;
	uint32_t ecx;
	uint32_t edx;
	uint32_t ebx;
	uint32_t esp;
	uint32_t ebp;
	uint32_t esi;
	uint32_t edi;
	uint32_t es;
	uint32_t cs;
	uint32_t ss;
	uint32_t ds;
	uint32_t fs;
	uint32_t gs;
	uint32_t ldtr;
	uint32_t trap;
	uint32_t iobase;
} tss_t;

void init_tss();
tss_t *get_tss();
void update_tss_info(unsigned long task_addr);

#endif	/*_X86_CPU_H*/