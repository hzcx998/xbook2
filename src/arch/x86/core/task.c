#include "task.h"
#include "interrupt.h"
#include "segment.h"

void __user_trap_frame_init(trap_frame_t *frame)
{
    /* 数据段 */
    frame->ds = frame->es = \
    frame->fs = frame->gs = USER_DATA_SEL; 

    /* 代码段 */
    frame->cs = USER_CODE_SEL;

    /* 栈段 */
    frame->ss = USER_STACK_SEL;

    /* 设置通用寄存器的值 */
    frame->edi = frame->esi = \
    frame->ebp = frame->esp_dummy = 0;

    frame->eax = frame->ebx = \
    frame->ecx = frame->edx = 0;
    
    /* 为了能让程序首次运行，这里需要设置eflags的值 */
    frame->eflags = (EFLAGS_MBS | EFLAGS_IF_1 | EFLAGS_IOPL_0);
}

void __ktask_trap_frame_init(trap_frame_t *frame)
{
    /* 数据段 */
    frame->ds = frame->es = \
    frame->fs = frame->gs = SERVE_DATA_SEL; 

    /* 代码段 */
    frame->cs = SERVE_CODE_SEL;

    /* 栈段 */
    frame->ss = SERVE_STACK_SEL;

    /* 设置通用寄存器的值 */
    frame->edi = frame->esi = \
    frame->ebp = frame->esp_dummy = 0;

    frame->eax = frame->ebx = \
    frame->ecx = frame->edx = 0;

    /* 为了能让程序首次运行，这里需要设置eflags的值 */
    frame->eflags = (EFLAGS_MBS | EFLAGS_IF_1 | EFLAGS_IOPL_1);
}

void __kernel_trap_frame_init(trap_frame_t *frame)
{
    /* 数据段 */
    frame->ds = frame->es = \
    frame->fs = frame->gs = KERNEL_DATA_SEL; 
    
    /* 代码段 */
    frame->cs = KERNEL_CODE_SEL;

    /* 栈段 */
    frame->ss = KERNEL_STACK_SEL;

    /* 设置通用寄存器的值 */
    frame->edi = frame->esi = \
    frame->ebp = frame->esp_dummy = 0;

    frame->eax = frame->ebx = \
    frame->ecx = frame->edx = 0;

    /* 为了能让程序首次运行，这里需要设置eflags的值 */
    frame->eflags = (EFLAGS_MBS | EFLAGS_IF_1 | EFLAGS_IOPL_1);
    printk("$ 4");
}
