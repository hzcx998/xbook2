#ifndef _XBOOK_TASK_H
#define _XBOOK_TASK_H

#include <arch/page.h>
#include <arch/task.h>
#include "types.h"
#include "list.h"
#include "vmm.h"

/* task state */
typedef enum task_state {
    TASK_READY = 0,         /* 进程处于就绪状态 */
    TASK_RUNNING,           /* 进程正在运行中 */
    TASK_BLOCKED,           /* 进程由于某种原因被阻塞 */
    TASK_WAITING,           /* 进程处于等待子进程状态 */
    TASK_STOPPED,           /* 进程处于停止运行状态 */
    TASK_HANGING,           /* 进程处于挂起，等待父进程来回收  */
    TASK_ZOMBIE,            /* 进程处于僵尸状态，父进程没有等待它 */
    TASK_DIED,              /* 进程处于死亡状态，资源已经被回收 */
} task_state_t;

#define MAX_TASK_NAMELEN 32

/* 栈魔数，用于检测内核栈是否向下越界 */
#define TASK_STACK_MAGIC 0X19980325

/* 栈中保存的最大参数个数 */
#define MAX_STACK_ARGC 16

/* 内核栈大小为8kb */
#define TASK_KSTACK_SIZE    4096

/* init 进程的pid */
#define INIT_PROC_PID       1

typedef struct priority_queue {
    list_t list;            /* 任务链表 */
    unsigned long length;   /* 队列长度 */
    unsigned int priority;  /* 优先级 */
} priority_queue_t;

typedef struct task {
    unsigned char *kstack;                // kernel stack, must be first member
    pid_t pid;                      // 自己的进程id
    pid_t parent_pid;                // 父进程id
    task_state_t state;          /* 任务的状态 */
    unsigned long priority;              /* 任务所在的优先级队列 */
    unsigned long ticks;                 /* 运行的ticks，当前剩余的timeslice */
    unsigned long block_ticks;                 /* 阻塞时的ticks数 */
    unsigned long timeslice;             /* 时间片，可以动态调整 */
    unsigned long elapsed_ticks;         /* 任务执行总共占用的时间片数 */
    int exit_status;                     // 退出时的状态
    char name[MAX_TASK_NAMELEN];
    struct vmm *vmm;                     /* 虚拟内存管理 */
    list_t list;               // 处于所在队列的链表
    list_t global_list;         // 全局任务队列，用来查找所有存在的任务
    priority_queue_t *prio_queue;   /* 所在的优先级队列 */
    unsigned int stack_magic;         /* 任务的魔数 */
} task_t;

#define SET_TASK_STATUS(task, stat) \
        (task)->state = stat

extern task_t *task_current;   /* 当前任务指针 */
extern list_t task_global_list;

/* 获取当前地址位置 */
#define __current_task()   ((task_t *)(current_task_addr)())

//#define current_task   __current_task()
#define current_task    task_current

void init_tasks();
void kernel_pause();

void task_init(task_t *task, char *name, int priority);
void task_free(task_t *task);

void dump_task(task_t *task);

void make_task_stack(task_t *task, task_func_t function, void *arg);
task_t *kthread_start(char *name, int priority, task_func_t func, void *arg);

void kthread_exit(task_t *task);

task_t *find_task_by_pid(pid_t pid);
void task_global_list_add(task_t *task);

void task_activate(task_t *task);

void task_block(task_state_t state);
void task_unblock(task_t *task);

#define task_sleep() task_block(TASK_BLOCKED) 
#define task_wakeup(task) task_unblock(task) 

task_t *process_create(char *name, char **argv);

pid_t fork_pid();
void print_task();



#endif   /* _XBOOK_TASK_H */
