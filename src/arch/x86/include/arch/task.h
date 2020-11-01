#ifndef _X86_TASK_H
#define _X86_TASK_H


#include <types.h>
#include <stdint.h>
#include "interrupt.h"
#include <xbook/task.h>

typedef struct {
    uint32_t ebp;
    uint32_t ebx;
    uint32_t edi;
    uint32_t esi;

    /* 首次运行指向kthread_func_t，其它时候指向switch_to的返回地址 */
    void (*eip) (task_func_t *func, void *arg);
    uint32_t unused;
    task_func_t *function;
    void *arg;
} thread_stack_t;

void thread_switch_to_next(void *prev, void *next);
void thread_kstack_dump(thread_stack_t *kstack);

void kernel_switch_to_user(trap_frame_t *frame);
void kernel_frame_init(trap_frame_t *frame);

#define user_set_entry_point(frame, entry) (frame)->eip = entry
void user_frame_init(trap_frame_t *frame);
void user_thread_frame_build(trap_frame_t *frame, void *arg, void *func,
    void *thread_entry, void *stack_top);

void trigger_frame_build(trap_frame_t *frame, int trig, void *act);
int trigger_return_to_user(trap_frame_t *frame);

void task_stack_build(task_t *task, task_func_t *function, void *arg);
int task_stack_build_when_forking(task_t *child);

int process_frame_init(task_t *task, trap_frame_t *frame, char **argv, char **envp);

#endif  /* _X86_TASK_H */
