#include <xbook/task.h>
#include <xbook/schedule.h>
#include <xbook/process.h>
#include <arch/interrupt.h>

#define DEBUG_LOCAL 1

/* 用户线程 */

/*
typedef sturct __xthread_attr {


} xthread_attr_t;


void *thread_routine(void *arg)
{


    return (void *) 0;
}

xthraed_t tid = xthread_start(thread_routine, arg, xthread_attr);

retval = xthread_exit((void *) 123);

retval = xthread_join(tig, (void *) status);

tid = sys_uthread_start(routine, arg);
sys_uthread_exit(tid);
*/

/**
 * uthread_entry - 用户线程内核的入口
 * @arg: 参数
 * 
 * 通过这个入口，可以跳转到用户态运行。
 * 
 */
void uthread_entry(void *arg) 
{
    printk(KERN_DEBUG "uthread_entry: ready into user thread.\n");
    trap_frame_t *frame = GET_TASK_TRAP_FRAME(current_task);
    switch_to_user(frame);
}

#if 0
/**
 * uthread_entry - 用户线程内核的入口
 * @arg: 参数
 * 
 * 构建c语言函数栈。
 * 
 * void *thread_start(* arg) {
 *  ...
 * }
 * 堆栈看起来是这样的：
 * 
 * esp + 8: arg 参数
 * esp + 4: caller addr 调用者返回地址
 * esp    : free stack top 可用栈顶
 */
void *uthread_entry(void *arg) 
{
    printk(KERN_DEBUG "uthread_entry: ready into user thread.\n");
    
    /* 获取当前任务的中断栈框 */
    trap_frame_t *frame = GET_TASK_TRAP_FRAME(current_task);   
    frame->esp -= sizeof(unsigned int); /* 预留参数（4字节） */
    unsigned int *stack_top = (unsigned int *)frame->esp;
    *stack_top = (unsigned int *)arg;   /* 往堆栈写入参数 */
    frame->esp -= sizeof(unsigned int); /* 预留调用者返回地址 */
    
    printk(KERN_DEBUG "uthread_entry: esp=%x eip=%x arg=%x stack=%x\n", frame->esp, frame->eip, arg, *stack_top);
    switch_to_user(frame);
}
#endif

/**
 * uthread_start - 开始一个用户线程
 * @func: 线程入口
 * @arg: 线程参数
 * 
 * 1.进程需要分配线程的堆栈
 * 2.需要传入线程入口
 * 3.需要传入线程例程和参数
 */
task_t *uthread_start(task_func_t *func, void *arg, 
    uthread_attr_t *attr, void *thread_entry)
{
    /* 创建线程的父进程 */
    task_t *parent = current_task;
#if DEBUG_LOCAL == 1
    printk(KERN_DEBUG "uthread_start: routine=%x arg=%x stackaddr=%x stacksize=%x detach=%d\n",
        func, arg, attr->stackaddr, attr->stacksize, attr->detachstate);
#endif
    // 创建一个新的线程结构体
    task_t *task = (task_t *) kmalloc(TASK_KSTACK_SIZE);
    
    if (!task)
        return NULL;
    
    // 初始化线程
    task_init(task, "uthread", TASK_PRIO_USER);
    task->tgid = parent->pid;   /* 线程组id指向父进程的pid */
    task->parent_pid = parent->pid; /* 父进程是创建者进程 */

#if DEBUG_LOCAL == 1
    printk(KERN_DEBUG "uthread_start: pid=%x tgid=%x parent pid=%d\n",
        task->pid, task->tgid, task->parent_pid);
#endif

    task->vmm = parent->vmm;    /*共享内存 */
    task->res = parent->res;    /* 共享资源 */
    task->triggers = parent->triggers;/* 共享触发器 */

    /* 中断栈框 */
    proc_make_trap_frame(task);

    // 创建一个线程
    make_task_stack(task, uthread_entry, arg);

#if 0
    /* 写入关键信息 */
    trap_frame_t *frame = GET_TASK_TRAP_FRAME(task);
    frame->eip = func;
    frame->esp = stack_top;
#endif
    /* 构建用户线程栈框 */
    trap_frame_t *frame = GET_TASK_TRAP_FRAME(task);
    build_user_thread_frame(frame, arg, (void *)func, thread_entry, 
        (unsigned char *)attr->stackaddr + attr->stacksize);

    if (attr->detachstate) {    /* 设置detach分离 */
        task->flags |= TASK_FLAG_DETACH;
    }
    
    /* 操作链表时关闭中断，结束后恢复之前状态 */
    unsigned long flags;
    save_intr(flags);

    task_global_list_add(task);
    task_priority_queue_add_tail(task);
    
    restore_intr(flags);
    return task;
}

uthread_t sys_thread_create(
    uthread_attr_t *attr,
    task_func_t *func,
    void *arg,
    void *thread_entry
){
    /* 传进来的属性为空就返回 */
    if (attr == NULL)
        return -1;

    task_t *task = uthread_start(func, arg, attr, thread_entry);
    if (task == NULL)
        return -1;  /* failed */
    return task->pid;       /* 返回线程的id */
}

void sys_thread_exit(void *retval)
{
    printk(KERN_DEBUG "sys_thread_exit: exit with %x\n", retval);
    
}
