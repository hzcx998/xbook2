#ifndef _X86_TASK_H
#define _X86_TASK_H

#include <xbook/stddef.h>
#include <xbook/memops.h>
#include "tss.h"

/* task local stack struct */
typedef struct task_local_stack {
    unsigned int ebp;
    unsigned int ebx;
    unsigned int edi;
    unsigned int esi;

    /* 首次运行指向do_task_func，其它时候指向switch_to的返回地址 */
    void (*eip) (task_func_t *func, void *arg);

    unsigned int unused_retaddr;
    task_func_t *function;   // 线程要调用的函数
    void *arg;  // 线程携带的参数
} task_local_stack_t;

/* init task local stack */
#define __init_task_lock_stack(stack, entry, func, arg) \
        do {                                            \
            stack->eip = entry;                         \
            stack->function = func;                     \
            stack->arg = arg;                           \
            stack->ebp = stack->ebx =                   \
            stack->esi = stack->edi = 0;                \
        } while (0);

/**
 * __current_task_addr - 获取当前运行任务的地址
 * 
 * 通过esp来计算出任务结构体
 */
static inline unsigned long *__current_task_addr()
{
    unsigned long esp;
    // 获取esp的值
    asm ("mov %%esp, %0" : "=g" (esp));
    /* 
    由于是在内核态，所以esp是内核态的值
    取esp整数部分并且减去一个PAGE即pcb起始地址
    内核栈，我们约定2个页的大小
    */
    /* 4k对齐，直接 esp & ~(4096UL-1)
        8k对齐却不能直接这么做，需要减4k后，再这么做。
        由于，任务的地址都是位于高端地址，所以减4k不会为负。
     */
    return (unsigned long *)((esp - PAGE_SIZE) & ~(PAGE_SIZE - 1));
}

/* task switch func */
void __switch_to(unsigned long prev, unsigned long next);

#endif	/* _X86_TASK_H */