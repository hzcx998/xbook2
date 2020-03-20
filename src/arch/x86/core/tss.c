#include "tss.h"
#include "segment.h"
#include "registers.h"
#include <xbook/memops.h>

/* tss对象 */
tss_t tss;

tss_t *get_tss()
{
	return &tss;
}

#if 0
void UpdateTssInfo(struct Task *task)
{
	// 更新tss.esp0的值为任务的内核栈顶
	tss.esp0 = (unsigned int)((uint32_t)task + TASK_KSTACK_SIZE);
}
#endif

void init_tss()
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
