#ifndef _XBOOK_TASK_H
#define _XBOOK_TASK_H

#include <arch/page.h>
#include <arch/task.h>
#include "types.h"
#include "list.h"
#include "vmm.h"
#include "trigger.h"
#include "resource.h"
#include "timer.h"
#include "alarm.h"
#include "pthread.h"

/* task state */
typedef enum task_state {
    TASK_READY = 0,         /* 进程处于就绪状态 */
    TASK_RUNNING,           /* 进程正在运行中 */
    TASK_BLOCKED,           /* 进程由于某种原因被阻塞 */
    TASK_WAITING,           /* 进程处于等待子进程状态 */
    TASK_STOPPED,           /* 进程处于停止运行状态 */
    TASK_HANGING,           /* 进程处于挂起，等待父进程来回收  */
    TASK_ZOMBIE,            /* 进程处于僵尸状态，父进程没有等待它 */
} task_state_t;

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

#define MAX_TASK_NAMELEN 32

/* 栈魔数，用于检测内核栈是否向下越界 */
#define TASK_STACK_MAGIC 0X19980325

/* 栈中保存的最大参数个数 */
#define MAX_STACK_ARGC 16

/* 内核栈大小为8kb */
#define TASK_KSTACK_SIZE    4096

/* init 进程的pid */
#define INIT_PROC_PID       1

/* 线程的标志 */
enum thread_flags {
    THREAD_FLAG_DETACH              = (1 << 0),     /* 线程分离标志，表示自己释放资源 */
    THREAD_FLAG_JOINED              = (1 << 1),     /* 线程被其它线程等待中 */
    THREAD_FLAG_JOINING             = (1 << 2),     /* 线程正在等待其它线程 */
    THREAD_FLAG_CANCEL_DISABLE      = (1 << 3),     /* 线程不能被取消 */
    THREAD_FLAG_CANCEL_ASYCHRONOUS  = (1 << 4),     /* 线程收到取消信号时立即退出 */
    THREAD_FLAG_CANCELED            = (1 << 5),     /* 线程已经标记上取消点 */
};

typedef struct priority_queue {
    list_t list;            /* 任务链表 */
    unsigned long length;   /* 队列长度 */
    unsigned int priority;  /* 优先级 */
} priority_queue_t;

typedef struct task {
    unsigned char *kstack;              /* kernel stack, must be first member */
    task_state_t state;                 /* 任务的状态 */
    pid_t pid;                          /* 自己的进程id */
    pid_t parent_pid;                   /* 父进程id */
    pid_t tgid;                         /* 线程组id：线程属于哪个进程，和pid一样，就说明是主线程，不然就是子线程 */
    unsigned long flags;                /* 标志 */
    unsigned long priority;             /* 任务所在的优先级队列 */
    unsigned long ticks;                /* 运行的ticks，当前剩余的timeslice */
    unsigned long timeslice;            /* 时间片，可以动态调整 */
    unsigned long elapsed_ticks;        /* 任务执行总共占用的时间片数 */
    int exit_status;                    /* 退出时的状态 */
    char name[MAX_TASK_NAMELEN];        /* 任务的名字 */
    struct vmm *vmm;                    /* 进程虚拟内存管理 */
    list_t list;                        /* 处于所在队列的链表 */
    list_t global_list;                 /* 全局任务队列，用来查找所有存在的任务 */
    priority_queue_t *prio_queue;       /* 所在的优先级队列 */
    triggers_t *triggers;               /* 触发器, 内核线程没有触发器 */
    resource_t *res;                    /* 设备资源 */
    timer_t *sleep_timer;               /* 休眠时的定时器 */
    alarm_t alarm;                      /* 闹钟 */
    long errno;                         /* 错误码：用户多线程时用来标记每一个线程的错误码 */
    pthread_desc_t *pthread;            /* 用户线程管理，多个线程共同占有，只有一个主线程的时候为NULL */
    unsigned int stack_magic;           /* 任务的魔数 */
} task_t;

extern task_t *task_current;   /* 当前任务指针 */
extern list_t task_global_list;

/* 获取当前地址位置 */
#define __current_task()   ((task_t *)(current_task_addr)())

#define current_task    task_current

#define set_current_state(stat) \
        current_task->state = (stat)

#define GET_TASK_TRAP_FRAME(task) \
        ((trap_frame_t *) (((unsigned char *) (task) + \
        TASK_KSTACK_SIZE) - sizeof(trap_frame_t)))

/* 判断任务是否为主线程 */
#define IN_SAME_THREAD_GROUP(a, b) \
        ((a)->tgid == (b)->tgid)

/* 判断任务是否为单线程 */
#define IN_SINGAL_THREAD(task) \
        (((task)->pthread && (atomic_get(&(task)->pthread->thread_count) == 0))  || \
        (task)->pthread == NULL)

/* 检测线程处于取消点 */
#define CHECK_THREAD_CANCELATION_POTINT(task) \
    do { \
        if (!((task)->flags & THREAD_FLAG_CANCEL_DISABLE) && \
            (task)->flags & THREAD_FLAG_CANCELED) { \
            pthread_exit((void *) THREAD_FLAG_CANCELED); \
        } \
    } while (0)
    
        
void init_tasks();
void kernel_pause();

void task_init(task_t *task, char *name, int priority);
void task_free(task_t *task);

void dump_task(task_t *task);

void make_task_stack(task_t *task, task_func_t *function, void *arg);
task_t *kthread_start(char *name, int priority, task_func_t *func, void *arg);
void kthread_exit(int status);

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

void start_user();

void dump_task_kstack(thread_stack_t *kstack);

pid_t sys_get_pid();
pid_t sys_get_ppid();
pid_t sys_get_tid();

unsigned long sys_sleep(unsigned long second);

void close_one_thread(task_t *thread);
void close_other_threads(task_t *thread);
void pthread_exit(void *status);

#endif   /* _XBOOK_TASK_H */
