#ifndef _XBOOK_TASK_H
#define _XBOOK_TASK_H

#include <xbook/types.h>
#include <xbook/list.h>
#include <arch/page.h>
#include <arch/task.h>

/* task state */
typedef enum task_state {
    TASK_READY = 0,         /* 进程处于就绪状态 */
    TASK_RUNNING,           /* 进程正在运行中 */
    TASK_BLOCKED,           /* 进程由于某种原因被阻塞 */
    TASK_WAITING,           /* 进程处于等待子进程状态 */
    TASK_STOPPED,           /* 进程处于停止运行状态 */
    TASK_ZOMBIE,            /* 进程处于僵尸状态，退出运行 */
    TASK_DIED,              /* 进程处于死亡状态，资源已经被回收 */
} task_state_t;

#define MAX_TASK_NAMELEN 32

/* 栈魔数，用于检测内核栈是否向下越界 */
#define TASK_STACK_MAGIC 0X19980325

/* 栈中保存的最大参数个数 */
#define MAX_STACK_ARGC 16

/* 内核栈大小为8kb */
#define TASK_KSTACK_SIZE    8192

typedef struct priority_queue {
    list_t list;            /* 任务链表 */
    unsigned long length;   /* 队列长度 */
    unsigned int priority;  /* 优先级 */
} priority_queue_t;

typedef struct vmm {
    void *page_storage;        /* 虚拟内存管理的结构 */                   
    
} vmm_t;

typedef struct task {
    unsigned char *kstack;                // kernel stack, must be first member
    pid_t pid;                      // 自己的进程id
    pid_t parent_pid;                // 父进程id
    task_state_t state;          /* 任务的状态 */
    unsigned long priority;              /* 任务所在的优先级队列 */
    unsigned long ticks;                 /* 运行的ticks，当前剩余的timeslice */
    unsigned long timeslice;             /* 时间片，可以动态调整 */
    unsigned long elapsed_ticks;         /* 任务执行总共占用的时间片数 */
    int exit_state;                     // 退出时的状态
    char name[MAX_TASK_NAMELEN];
    vmm_t *vmm;                     /* 虚拟内存管理 */
    list_t list;               // 处于所在队列的链表
    list_t global_list;         // 全局任务队列，用来查找所有存在的任务
    priority_queue_t *prio_queue;   /* 所在的优先级队列 */
    struct task *next;  /* 指向下一个任务的指针 */
    unsigned int stack_magic;         /* 任务的魔数 */
} task_t;

#define SET_TASK_STATUS(task, stat) \
        (task)->state = stat

/* 获取当前地址位置 */
#define __current_task()   (task_t *)(current_task_addr)()

#define current_task   __current_task()

void init_tasks();
void kernel_pause();

void dump_task(task_t *task);

task_t *kthread_start(char *name, int priority, task_func_t func, void *arg);

void kthread_exit(task_t *task);

task_t *find_task_by_id(pid_t pid);
void task_gloabl_list_add(task_t *task);

void task_activate(task_t *task);
void page_dir_active(task_t *task);

unsigned long *create_vmm_frame();

void task_block(task_state_t state);
void task_unblock(task_t *task);

#define task_sleep() task_block(TASK_BLOCKED) 
#define task_wakeup(task) task_unblock(task) 

#endif   /* _XBOOK_TASK_H */
