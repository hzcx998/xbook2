#include "task.h"
#include "interrupt.h"
#include "segment.h"
#include <xbook/syscall.h>
#include <xbook/trigger.h>

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
}


void __build_trigger_frame(int trig, void *_act, trap_frame_t *frame)
{
    trig_action_t *act = (trig_action_t *)_act;

    /* 获取信号栈框，在用户的esp栈下面 */
    trigger_frame_t *trigger_frame = (trigger_frame_t *)((frame->esp - sizeof(trigger_frame_t)) & -8UL);

    /*printk("trap frame %x, signal frame esp %x, %x\n", 
    frame->esp, frame->esp - sizeof(trigger_frame_t), (frame->esp - sizeof(trigger_frame_t)) & -8UL);
    */
    /* 传递给handler的参数 */
    trigger_frame->trig = trig;
    
    /* 把中断栈保存到信号栈中 */
    memcpy(&trigger_frame->trap_frame, frame, sizeof(trap_frame_t));

    /* 设置返回地址为构建的返回代码 */
    trigger_frame->ret_addr = trigger_frame->ret_code;

    /* 构建返回代码，系统调用封装
    模拟系统调用来实现从用户态返回到内核态
    mov eax, SYS_TRIGRET
    int 0x40
     */
    trigger_frame->ret_code[0] = 0xb8; /* 给eax赋值的机器码 */
    *(int *)(trigger_frame->ret_code + 1) = SYS_TRIGRET;    /* 把系统调用号填进去 */
    *(short *)(trigger_frame->ret_code + 5) = 0x40cd;      /* int对应的指令是0xcd，系统调用中断号是0x40 */
    
    /* 设置中断栈的eip成为用户设定的处理函数 */
    frame->eip = (unsigned long)act->handler;

    /* 设置运行时的栈 */
    frame->esp = (unsigned long)trigger_frame;

    /* 设置成用户态的段寄存器 */
    frame->ds = frame->es = frame->fs = frame->gs = USER_DATA_SEL;
    frame->ss = USER_STACK_SEL;
    frame->cs = USER_CODE_SEL;
}

void __build_user_thread_frame(trap_frame_t *frame, void *arg, void *func,
    void *thread_entry, void *stack_top)
{
    frame->ebx = (unsigned long) arg;     /* 参数 */
    frame->ecx = (unsigned long) func;    /* 线程例程入口 */
    frame->eip = (unsigned long) thread_entry;
    frame->esp = (unsigned long) stack_top;
}
    