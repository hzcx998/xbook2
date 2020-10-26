#include <arch/tss.h>
#include <arch/segment.h>
#include <arch/registers.h>
#include <arch/pmem.h>
#include <string.h>
#include <xbook/task.h>

/* tss对象 */
tss_t tss;

tss_t *tss_get_from_cpu0()
{
	return &tss;
}

void update_tss_info(unsigned long task_addr)
{
	// 更新tss.esp0的值为任务的内核栈顶
	tss.esp0 = (unsigned long)(task_addr + TASK_KSTACK_SIZE);
}

void tss_init()
{
	memset(&tss, 0, sizeof(tss));
	// 内核的内核栈
	tss.esp0 = KERNEL_STATCK_TOP;
	// 内核栈选择子
	tss.ss0 = KERNEL_DATA_SEL;
    
	tss.iobase = sizeof(tss);
	// 加载tss register
	load_tr(KERNEL_TSS_SEL);
}
