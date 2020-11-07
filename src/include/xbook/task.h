#ifndef _XBOOK_TASK_H
#define _XBOOK_TASK_H

#include <arch/page.h>
#include <arch/cpu.h>
#include <sys/proc.h>
#include <types.h>
#include <xbook/list.h>
#include "vmm.h"
#include "trigger.h"
#include "timer.h"
#include "alarm.h"
#include "pthread.h"
#include "fs.h"
#include "msgpool.h"
#include "spinlock.h"

typedef enum {
    TASK_READY = 0,         /* 进程处于就绪状态 */
    TASK_RUNNING,           /* 进程正在运行中 */
    TASK_BLOCKED,           /* 进程由于某种原因被阻塞 */
    TASK_WAITING,           /* 进程处于等待子进程状态 */
    TASK_STOPPED,           /* 进程处于停止运行状态 */
    TASK_HANGING,           /* 进程处于挂起，等待父进程来回收  */
    TASK_ZOMBIE,            /* 进程处于僵尸状态，父进程没有等待它 */
} task_state_t;

#define MAX_TASK_NAMELEN 32
#define TASK_STACK_MAGIC 0X19980325
#define MAX_TASK_STACK_ARG_NR 16
#define TASK_KERN_STACK_SIZE    8192

#define USER_INIT_PROC_ID       1

#define TASK_TIMESLICE_MIN  1
#define TASK_TIMESLICE_MAX  100

enum thread_flags {
    THREAD_FLAG_DETACH              = (1 << 0),     /* 线程分离标志，表示自己释放资源 */
    THREAD_FLAG_JOINED              = (1 << 1),     /* 线程被其它线程等待中 */
    THREAD_FLAG_JOINING             = (1 << 2),     /* 线程正在等待其它线程 */
    THREAD_FLAG_CANCEL_DISABLE      = (1 << 3),     /* 线程不能被取消 */
    THREAD_FLAG_CANCEL_ASYCHRONOUS  = (1 << 4),     /* 线程收到取消信号时立即退出 */
    THREAD_FLAG_CANCELED            = (1 << 5),     /* 线程已经标记上取消点 */
};

typedef struct {
    unsigned char *kstack;              /* kernel stack, must be first member */
    task_state_t state;
    spinlock_t lock;
    cpuid_t cpuid;
    pid_t pid;                          /* process id */
    pid_t parent_pid;
    pid_t tgid;                         /* 线程组id：线程属于哪个进程，和pid一样，就说明是主线程，不然就是子线程 */
    unsigned long flags;                
    unsigned long priority;             /* 任务的动态优先级 */
    unsigned long static_priority;      /* 任务的静态优先级 */
    unsigned long ticks;                /* 运行的ticks，当前剩余的timeslice */
    unsigned long timeslice;            /* 时间片，可以动态调整 */
    unsigned long elapsed_ticks;        /* 任务执行总共占用的时间片数 */
    int exit_status;                    
    char name[MAX_TASK_NAMELEN];        
    struct vmm *vmm;                    
    list_t list;                        /* 处于所在队列的链表，就绪队列，阻塞队列等 */
    list_t global_list;                 /* 全局任务队列，用来查找所有存在的任务 */
    triggers_t *triggers;               
    timer_t sleep_timer;               
    alarm_t alarm;                      
    long errno;                         /* 错误码：用户多线程时用来标记每一个线程的错误码 */
    pthread_desc_t *pthread;            /* 用户线程管理，多个线程共同占有，只有一个主线程的时候为NULL */
    file_man_t *fileman;                
    msgpool_t *gmsgpool;
    unsigned int stack_magic;
} task_t;

extern list_t task_global_list;
extern volatile int task_init_done;

#define TASK_GET_TRAP_FRAME(task) \
        ((trap_frame_t *) (((unsigned char *) (task) + \
        TASK_KERN_STACK_SIZE) - sizeof(trap_frame_t)))

#define TASK_IN_SAME_THREAD_GROUP(a, b) \
        ((a)->tgid == (b)->tgid)

#define TASK_IS_SINGAL_THREAD(task) \
        (((task)->pthread && (atomic_get(&(task)->pthread->thread_count) <= 1))  || \
        (task)->pthread == NULL)

#define TASK_CHECK_THREAD_CANCELATION_POTINT(task) \
    do { \
        if (!((task)->flags & THREAD_FLAG_CANCEL_DISABLE) && \
            (task)->flags & THREAD_FLAG_CANCELED) { \
            pthread_exit((void *) THREAD_FLAG_CANCELED); \
        } \
    } while (0)
      
void tasks_init();

void task_init(task_t *task, char *name, int priority);
void task_free(task_t *task);
void task_dump(task_t *task);

task_t *kern_thread_start(char *name, int priority, task_func_t *func, void *arg);
void kern_thread_exit(int status);

task_t *task_find_by_pid(pid_t pid);
void task_add_to_global_list(task_t *task);
void task_activate_when_sched(task_t *task);

void task_block(task_state_t state);
void task_unblock(task_t *task);
void task_yeild();

void task_set_timeslice(task_t *task, uint32_t timeslice);

#define task_sleep() task_block(TASK_BLOCKED) 
static inline void task_wakeup(task_t *task)
{
    if ((task->state == TASK_BLOCKED) || 
        (task->state == TASK_WAITING) ||
        (task->state == TASK_STOPPED)) {    
        task_unblock(task);
    }
}

pid_t task_take_pid();
void task_rollback_pid();
void tasks_print();
void task_start_user();
unsigned long task_sleep_by_ticks(clock_t ticks);
int task_count_children(task_t *parent);
int task_do_cancel(task_t *task);

#define sys_sched_yeild     task_yeild
pid_t sys_get_pid();
pid_t sys_get_ppid();
pid_t sys_get_tid();
int sys_getver(char *buf, int len);
int sys_tstate(tstate_t *ts, unsigned int *idx);

#endif   /* _XBOOK_TASK_H */
