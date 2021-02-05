#include <arch/tss.h>
#include <arch/segment.h>
#include <arch/registers.h>
#include <arch/phymem.h>
#include <string.h>
#include <xbook/task.h>

tss_t tss0;

tss_t *tss_get_from_cpu0()
{
	return &tss0;
}

void tss_update_info(unsigned long task_addr)
{
	// 更新tss.esp0的值为任务的内核栈顶
	tss0.esp0 = (unsigned long)(task_addr + TASK_KERN_STACK_SIZE);
}

void tss_init()
{
	memset(&tss0, 0, sizeof(tss_t));
	tss0.esp0 = KERNEL_STATCK_TOP;
	tss0.ss0 = KERNEL_DATA_SEL;
	tss0.iobase = sizeof(tss_t);
	task_register_set(KERNEL_TSS_SEL);
}
