#ifndef _X86_TASK_H
#define _X86_TASK_H

#include <xbook/stddef.h>
#include <xbook/memops.h>
#include <xbook/debug.h>
#include "tss.h"
#include "interrupt.h"
#include "page.h"

/* x86线程栈结构 */
typedef struct thread_stack {
    uint32_t ebp;
    uint32_t ebx;
    uint32_t edi;
    uint32_t esi;

    /* 首次运行指向kthread_func_t，其它时候指向switch_to的返回地址 */
    void (*eip) (task_func_t *func, void *arg);

    uint32_t unused;
    task_func_t *function;   // 线程要调用的函数
    void *arg;  // 线程携带的参数
} thread_stack_t;


/* init task local stack */
#define __init_task_lock_stack(stack, entry, func, arg) \
        do {                                            \
            stack->eip = entry;                         \
            stack->function = func;                     \
            stack->arg = arg;                           \
            stack->ebp = stack->ebx =                   \
            stack->esi = stack->edi = 0;                \
        } while (0);

static inline unsigned long get_esp()
{
    unsigned long esp;
    // 获取esp的值
    asm ("mov %%esp, %0" : "=g" (esp));
    return esp;
}

/* task switch func */
void __switch_to(unsigned long prev, unsigned long next);

/* task switch from kernel mode to user mode */
void __switch_to_user(trap_frame_t *frame);

void __user_trap_frame_init(trap_frame_t *frame);
void __ktask_trap_frame_init(trap_frame_t *frame);
void __kernel_trap_frame_init(trap_frame_t *frame);

void __build_user_thread_frame(trap_frame_t *frame, void *arg, void *func,
    void *thread_entry, void *stack_top);

void __build_trigger_frame(int trig, void *act, trap_frame_t *frame);

#define __user_entry_point(frame, entry) (frame)->eip = entry

void intr_exit();
void intr_exit2();

#endif	/* _X86_TASK_H */