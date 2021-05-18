#ifndef __RISCV64_PROC_H
#define __RISCV64_PROC_H

#include <stdint.h>
#include <types.h>
#include <xbook/spinlock.h>
#include <xbook/task.h>
#include <arch/fpu.h>

#define NPROC 32

// Saved registers for kernel context switches.
struct context {
  uint64_t ra;
  uint64_t sp;

  // callee-saved
  uint64_t s0;
  uint64_t s1;
  uint64_t s2;
  uint64_t s3;
  uint64_t s4;
  uint64_t s5;
  uint64_t s6;
  uint64_t s7;
  uint64_t s8;
  uint64_t s9;
  uint64_t s10;
  uint64_t s11;
};

// Per-process state
typedef struct {
    uint8_t *kstack;               // Virtual address of kernel stack
    spinlock_t lock;
    // p->lock must be held when using these:
    task_state_t state;        // Process state
    pid_t pid;                     // Process ID
    unsigned long flags;
    char priority;             /* 任务的动态优先级 */
    char static_priority;      /* 任务的静态优先级 */
    unsigned long ticks;                /* 运行的ticks，当前剩余的timeslice */
    unsigned long timeslice;            /* 时间片，可以动态调整 */
    unsigned long elapsed_ticks;        /* 任务执行总共占用的时间片数 */
    list_t list;                        /* 处于所在队列的链表，就绪队列，阻塞队列等 */
    list_t global_list;                 /* 全局任务队列，用来查找所有存在的任务 */

    task_func_t *kthread_entry;
    void *kthread_arg;       // 线程执行时传入参数
    // these are private to the process, so p->lock need not be held.
    struct context context;      // swtch() here to run process
    char name[MAX_TASK_NAMELEN];
    fpu_t fpu;

    /* NOTE: 进程相关的结构 */
    pgdir_t page_storage;

    unsigned int stack_magic;
} proc_t;

void procinit(void);
void proc_yield();
void task_start_other();

void thread_switch_to_next(struct context *old, struct context *new);

void proc_activate_when_sched(proc_t *task);

int proc_pgdir_init(proc_t *proc);


#endif  /* __RISCV64_PROC_H */