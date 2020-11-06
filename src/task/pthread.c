#include <xbook/schedule.h>
#include <xbook/process.h>
#include <xbook/pthread.h>
#include <arch/interrupt.h>
#include <arch/task.h>

#define DEBUG_PTHREAD

/* 用户线程 */

void pthread_desc_init(pthread_desc_t *pthread)
{
    if (pthread != NULL) {
        /* 最开始只有一个主线程（当前进程） */
        atomic_set(&pthread->thread_count, 1);
    }
}

void pthread_desc_exit(pthread_desc_t *pthread)
{
    if (pthread != NULL)
        mem_free(pthread);
}

/**
 * pthread_entry - 用户线程内核的入口
 * @arg: 参数
 * 
 * 通过这个入口，可以跳转到用户态运行。
 * 
 */
void pthread_entry(void *arg) 
{
    trap_frame_t *frame = TASK_GET_TRAP_FRAME(task_current);
    kernel_switch_to_user(frame);
}


/**
 * thread_release_resource - 释放线程资源
 * @task: 任务
 * 
 * 线程资源比较少，主要是任务结构体（需要交给父进程处理），
 * 自己要处理的很少很少。。。
 * 
 * @return: 释放成功返回0，释放失败返回-1
 */
int thread_release_resource(task_t *task)
{
    timer_cancel(&task->sleep_timer);
    return 0;
}

/**
 * wait_one_hangging_thread - 处理一个挂起线程
 * 
 */
int wait_one_hangging_thread(task_t *parent, pid_t pid, int *status)
{
    task_t *child, *next;
    /* 可能删除列表元素，需要用safe */
    list_for_each_owner_safe (child, next, &task_global_list, global_list) {
        if (child->pid == pid) { /* find a child process we ordered */
            if (child->state == TASK_HANGING) { /* child is hanging, destroy it  */
#ifdef DEBUG_PTHREAD
                printk(KERN_NOTICE "wait_one_hangging_thread: pid=%d find a hanging thread %d \n",
                    parent->pid, pid);
#endif              
                /* 状态是可以为空的，不为空才写入子进程退出状态 */
                if (status != NULL)
                    *status = child->exit_status;
                /* 销毁子进程的PCB */
                proc_destroy(child, 1);
                return pid;
            }
        }
    }
    return -1;
}

/**
 * pthread_start - 开始一个用户线程
 * @func: 线程入口
 * @arg: 线程参数
 * 
 * 1.进程需要分配线程的堆栈
 * 2.需要传入线程入口
 * 3.需要传入线程例程和参数
 */
task_t *pthread_start(task_func_t *func, void *arg, 
    pthread_attr_t *attr, void *thread_entry)
{
    /* 创建线程的父进程 */
    task_t *parent = task_current;
#ifdef DEBUG_PTHREAD
    printk(KERN_DEBUG "pthread_start: pid=%d routine=%x arg=%x stackaddr=%x stacksize=%x detach=%d\n",
        parent->pid, func, arg, attr->stackaddr, attr->stacksize, attr->detachstate);
#endif

    if (parent->pthread == NULL) {  /* 第一次创建线程 */
        if (proc_pthread_init(parent))
            return NULL;    /* 初始化失败 */
    }

    // 创建一个新的线程结构体
    task_t *task = (task_t *) mem_alloc(TASK_KERN_STACK_SIZE);
    
    if (!task)
        return NULL;
    
    // 初始化线程
    task_init(task, "pthread", TASK_PRIO_USER);

    if (parent->tgid == parent->pid) {  /* 父进程是主线程 */
        task->tgid = parent->pid;   /* 线程组id指向父进程的pid */
    } else {    /* 父进程不是主线程，是子线程 */
        task->tgid = parent->tgid;   /* 线程组指向父进程的线程组 */
    }
    task->parent_pid = parent->pid; /* 父进程是创建者 */
    task->pthread = parent->pthread;    /* 指向父进程的线程管理 */
    
#ifdef DEBUG_PTHREAD
    printk(KERN_DEBUG "pthread_start: new thread pid=%d tgid=%x parent pid=%d\n",
        task->pid, task->tgid, task->parent_pid);
#endif

    task->vmm = parent->vmm;    /*共享内存 */
    task->triggers = parent->triggers; /* 共享触发器 */
    task->fileman = parent->fileman; /* 共享文件管理 */
    
    /* 中断栈框 */
    proc_trap_frame_init(task);

    // 创建一个线程
    task_stack_build(task, pthread_entry, arg);

#if 0
    /* 写入关键信息 */
    trap_frame_t *frame = TASK_GET_TRAP_FRAME(task);
    frame->eip = func;
    frame->esp = stack_top;
#endif
    /* 构建用户线程栈框 */
    trap_frame_t *frame = TASK_GET_TRAP_FRAME(task);
    user_thread_frame_build(frame, arg, (void *)func, thread_entry, 
        (unsigned char *)attr->stackaddr + attr->stacksize);

    if (attr->detachstate == PTHREAD_CREATE_DETACHED) {    /* 设置detach分离 */
        task->flags |= THREAD_FLAG_DETACH;
    }
    
    /* 操作链表时关闭中断，结束后恢复之前状态 */
    unsigned long flags;
    interrupt_save_and_disable(flags);

    atomic_inc(&task->pthread->thread_count);   /* 增加一个线程 */
    if (atomic_get(&task->pthread->thread_count) > PTHREAD_MAX_NR) { /* 超过最大线程数量，就不能创建 */
#ifdef DEBUG_PTHREAD
        printk(KERN_NOTICE "pthread_start: pid=%d tgid=%d the number of thread out of range!\n",
            parent->pid, parent->tgid);
#endif
        atomic_dec(&task->pthread->thread_count);
        mem_free(task);
        interrupt_restore_state(flags);
        return NULL;
    }
#ifdef DEBUG_PTHREAD
    printk(KERN_NOTICE "pthread_start: thread count %d\n", atomic_get(&task->pthread->thread_count));
#endif

    task_add_to_global_list(task);
    sched_queue_add_tail(sched_get_cur_unit(), task);
    
    interrupt_restore_state(flags);
    return task;
}

pid_t sys_thread_create(
    pthread_attr_t *attr,
    task_func_t *func,
    void *arg,
    void *thread_entry
){
    /* 传进来的属性为空就返回 */
    if (attr == NULL)
        return -1;

    task_t *task = pthread_start(func, arg, attr, thread_entry);
    if (task == NULL)
        return -1;  /* failed */
    return task->pid;       /* 返回线程的id */
}


void pthread_exit(void *status)
{
    unsigned long flags;
    interrupt_save_and_disable(flags);

    task_t *cur = task_current;

    /* 减少线程数量 */
    atomic_dec(&cur->pthread->thread_count);   
    if (atomic_get(&cur->pthread->thread_count) == 0) { /* 没有线程，就直接退出整个进程 */
        /* 退出整个进程 */
        printk(KERN_DEBUG "pthread_exit: pid=%d no other threads, exit process!\n", cur->pid);
        sys_exit((int) status);
    }
    
    cur->exit_status = (int)status;

    /* 释放内核资源 */
    thread_release_resource(cur);

    /* 子线程退出 */
    if (cur->flags & THREAD_FLAG_DETACH) {    /* 不需要同步等待，"自己释放资源" */
#ifdef DEBUG_PTHREAD
        printk(KERN_DEBUG "pthread_exit: pid=%d detached.\n", cur->pid);
#endif        
        /* 有可能启动时是joinable的，但是执行过程中变成detach状态，
        因此，可能存在父进程join等待，所以，这里就需要检测任务状态 */
        if (cur->flags &  THREAD_FLAG_JOINED) {    /* 处于join状态 */
            /* 父进程指向join中的进程 */
            task_t *parent = task_find_by_pid(cur->parent_pid);
            if (parent != NULL && parent->state == TASK_WAITING) {  /* 如果父进程在等待中 */
                if (parent->tgid == cur->tgid) {    /* 父进程和自己属于同一个线程组 */
                    printk(KERN_DEBUG "pthread_exit: pid=%d parent %s pid=%d joining, wakeup it.\n",
                        cur->pid, parent->name, parent->pid);
                    parent->flags &= ~THREAD_FLAG_JOINING;  /* 去掉等待中标志 */
                    /* 唤醒父进程 */
                    task_unblock(parent);
                }
            }
        }
        /* 过继给主线程 */
        //cur->parent_pid = cur->tgid;
        cur->parent_pid = USER_INIT_PROC_ID;

    } else {    /* 需要同步释放 */
#ifdef DEBUG_PTHREAD
        printk(KERN_DEBUG "pthread_exit: pid=%d joinable.\n", cur->pid);
#endif    
    }
#ifdef DEBUG_PTHREAD
    printk(KERN_DEBUG "pthread_exit: pid=%d tgid=%d ppid=%d.\n", cur->pid, cur->tgid, cur->parent_pid);
#endif
    task_t *parent = task_find_by_pid(cur->parent_pid); 
    if (parent) {
        /* 查看父进程状态 */
        if (parent->state == TASK_WAITING) {
            interrupt_restore_state(flags);
#ifdef DEBUG_PTHREAD
            printk(KERN_DEBUG "pthread_exit: pid=%d parent %d waiting...\n", cur->pid, parent->pid);
#endif    

            //printk("parent waiting...\n");
            task_unblock(parent); /* 唤醒父进程 */
            task_block(TASK_HANGING);   /* 把自己挂起 */
        } else { /* 父进程没有 */
            interrupt_restore_state(flags);
#ifdef DEBUG_PTHREAD
            printk(KERN_DEBUG "pthread_exit: pid=%d parent %d not waiting, zombie!\n", cur->pid, parent->pid);
#endif    
            //printk("parent not waiting, zombie!\n");
            task_block(TASK_ZOMBIE);   /* 变成僵尸进程 */
        }
    } else {
        /* 没有父进程，变成不可被收养的孤儿+僵尸进程 */
#ifdef DEBUG_PTHREAD
            printk(KERN_DEBUG "pthread_exit: pid=%d no parent! zombie!\n", cur->pid);
#endif    
        //printk("no parent!\n");
        interrupt_restore_state(flags);
        task_block(TASK_ZOMBIE); 
    }
}

void sys_thread_exit(void *retval)
{
    pthread_exit(retval);
}

/**
 * sys_thread_detach - 设置线程为分离状态
 * @thread: 线程
 * 
 */
int sys_thread_detach(pthread_t thread) 
{
    task_t *task = task_find_by_pid(thread);
    if (task == NULL)   /* not found */
        return -1;
    printk(KERN_DEBUG "sys_thread_detach: thread=%s pid=%d tgid=%d ppid=%d set detach.\n",
        task->name, task->pid, task->tgid, task->parent_pid);
    task->flags |= THREAD_FLAG_DETACH; /* 分离标志 */
    /* 有可能启动时是joinable的，但是执行过程中变成detach状态，
    因此，可能存在父进程join等待，所以，这里就需要检测任务状态 */
    if (task->flags & THREAD_FLAG_JOINED) {    /* 处于join状态 */
        /* 父进程指向join中的进程 */
        task_t *parent = task_find_by_pid(task->parent_pid);
        if (parent != NULL && parent->state == TASK_WAITING) {  /* 如果父进程在等待中 */
            if (parent->tgid == task->tgid) {    /* 父进程和自己属于同一个线程组 */
                printk(KERN_DEBUG "pthread_exit: pid=%d parent %s pid=%d joining, wakeup it.\n",
                    task->pid, parent->name, parent->pid);
                parent->flags &= ~THREAD_FLAG_JOINING;  /* 去掉等待中标志 */
                /* 唤醒父进程 */
                task_unblock(parent);
            }
        }
    }
    return 0;
}


int pthread_join(pthread_t thread, void **thread_return)
{
    task_t *waiter = task_current;  /* 当前进程是父进程 */
    unsigned long flags;
    interrupt_save_and_disable(flags);
    /* 先查看线程，是否存在，并且要是线程才行 */
    task_t *task, *find = NULL;
    list_for_each_owner (task, &task_global_list, global_list) {
        /* find the thread and not zombie */
        if (task->pid == thread && task->state != TASK_ZOMBIE) {
            find = task;    /* find thread */
            break;
        }
    }
    
    if (find == NULL) { /* 线程不存在 */
        interrupt_restore_state(flags);
        return -1;  /* 没找到线程 */
    }
#ifdef DEBUG_PTHREAD
    printk(KERN_DEBUG "pthread_join: pid=%d join thread %d\n", waiter->pid, thread);
#endif
    /* 线程存在，查看其是否为分离状态 */
    if (find->flags & THREAD_FLAG_DETACH) {
#ifdef DEBUG_PTHREAD
        printk(KERN_DEBUG "pthread_join: pid=%d join the %d was detached, just return.\n", waiter->pid, thread);
#endif  
        interrupt_restore_state(flags);
        return -1;
    }
 
    /* 么有线程等待中才能等待 */
    if (find->flags & THREAD_FLAG_JOINED) {
#ifdef DEBUG_PTHREAD
        printk(KERN_DEBUG "pthread_join: pid=%d the thread %d had joined by thread %d, return.\n",
            waiter->pid, find->pid, find->parent_pid);
#endif 
        interrupt_restore_state(flags);
        return -1;  /* 已经有一个线程在等待，不能等待 */
    }

    find->flags |= THREAD_FLAG_JOINED; /* 被线程等待 */
    find->parent_pid = waiter->pid;     /* 等待者变成父进程，等待子线程，可以通过thread_exit来唤醒父进程 */

    waiter->flags |= THREAD_FLAG_JOINING;   /* 处于等待中 */

    int status;
    pid_t pid;
    /* 当线程没有退出的时候就一直等待 */
    do {
        status = 0;
        pid = wait_one_hangging_thread(waiter, find->pid, &status);
        
#ifdef DEBUG_PTHREAD
        printk(KERN_DEBUG "pthread_join: pid=%d wait pid=%d status=%x\n",
            waiter->pid, pid, status);
#endif
        if (pid == thread) {
            break; /* 处理了指定的任务，就返回 */
        }
        /* 如果等待者joining状态取消了，就说明等待的线程在执行过程中变成DETACH状态，
        并且退出时来取消该标志 */
        if (!(waiter->flags & THREAD_FLAG_JOINING)) {
            break;
        }
#ifdef DEBUG_PTHREAD
        printk(KERN_DEBUG "pthread_join: pid=%d waiting...\n", waiter->pid);
#endif        
        interrupt_restore_state(flags);
        /* WATING for thread to exit */
        task_block(TASK_WAITING);
        
        interrupt_save_and_disable(flags);
    } while (pid == -1);
    /* 回写状态 */
    if (thread_return != NULL) {
        *thread_return = (void *)status;
#ifdef DEBUG_PTHREAD
        printk(KERN_DEBUG "pthread_join: pid=%d thread %d exit, will return status=%x\n",
            waiter->pid, thread, *thread_return);
#endif
    }

    interrupt_restore_state(flags);
    return 0;
}

/**
 * sys_thread_join - 等待线程退出
 * @thread: 线程
 * @thread_return: 返回值 
 */
int sys_thread_join(pthread_t thread, void **thread_return)
{
    TASK_CHECK_THREAD_CANCELATION_POTINT(task_current);
    return pthread_join(thread, thread_return);
}


/**
 * sys_thread_cancel - 设置线程取消点
 * @thread: 线程
 * 
 * 引起阻塞的系统调用都是Cancelation-point
 * 
 * 成功返回0，失败返回-1
 */
int sys_thread_cancel(pthread_t thread)
{
    task_t *task;
    unsigned long flags;
    interrupt_save_and_disable(flags);
    task = task_find_by_pid(thread);
    if (task == NULL) { /* 没找到 */
#ifdef DEBUG_PTHREAD
        printk(KERN_ERR "sys_thread_cancel: pid=%d not find thread %d!\n",
            task_current->pid, thread);
#endif 
        interrupt_restore_state(flags);
        return -1;
    }
#ifdef DEBUG_PTHREAD
    printk(KERN_ERR "sys_thread_cancel: pid=%d set thread %d cancelation point.\n",
        task_current->pid, thread);
#endif
    /* 设置取消点 */
    task->flags |= THREAD_FLAG_CANCELED;
    if (task->flags & THREAD_FLAG_CANCEL_ASYCHRONOUS) { /* 立即取消线程处理 */ 
        /* 查看是否为自己取消自己 */
        if (task == task_current) { /* 是自己 */
            interrupt_restore_state(flags);
#ifdef DEBUG_PTHREAD
            printk(KERN_DEBUG "sys_thread_cancel: pid=%d cancel self.\n", task_current->pid);
#endif 
            pthread_exit((void *) THREAD_FLAG_CANCEL_ASYCHRONOUS); /* 退出线程运行 */
        } else {    /* 取消其它线程 */
#ifdef DEBUG_PTHREAD
            printk(KERN_DEBUG "sys_thread_cancel: pid=%d will cancel thread %d.\n",
                task_current->pid, thread);
#endif 
            close_one_thread(task);
        }
    }
    interrupt_restore_state(flags);
    return 0;
}


/**
 * sys_thread_testcancel - 线程测试取消点
 * 
 * 如果有取消点，就退出线程
 */
void sys_thread_testcancel(void)
{
    TASK_CHECK_THREAD_CANCELATION_POTINT(task_current);
}
/**
 * sys_thread_setcancelstate - 设置取消状态
 * @state: 状态：PTHREAD_CANCEL_ENABLE（缺省）, 收到信号后设为CANCLED状态
 *              PTHREAD_CANCEL_DISABLE, 忽略CANCEL信号继续运行             
 * @oldstate: 原来的状态,old_state如果不为NULL则存入原来的Cancel状态以便恢复。 
 * 
 * 成功返回0，失败返回-1
 */
int sys_thread_setcancelstate(int state, int *oldstate)
{
    task_t *cur = task_current;
    if (oldstate != NULL) {  /* 保存原来的类型 */
#ifdef DEBUG_PTHREAD
        printk(KERN_DEBUG "sys_thread_setcancelstate: pid=%d fetch oldstate.\n",
            task_current->pid);
#endif 
        if (cur->flags & THREAD_FLAG_CANCEL_DISABLE) {
            *oldstate = PTHREAD_CANCEL_DISABLE;  
        } else {
            *oldstate = PTHREAD_CANCEL_ENABLE;
        }
    }
    if (state == PTHREAD_CANCEL_DISABLE) {
        cur->flags |= THREAD_FLAG_CANCEL_DISABLE;
#ifdef DEBUG_PTHREAD
        printk(KERN_DEBUG "sys_thread_setcancelstate: pid=%d set cancel disable.\n",
            task_current->pid);
#endif 
    } else if (state == PTHREAD_CANCEL_ENABLE) {
        cur->flags &= ~THREAD_FLAG_CANCEL_DISABLE;
#ifdef DEBUG_PTHREAD
        printk(KERN_DEBUG "sys_thread_setcancelstate: pid=%d set cancel enable.\n",
            task_current->pid);
#endif
    } else {
        return -1;
    }
    return 0;
}

/**
 * sys_thread_setcanceltype - 设置取消动作的执行时机
 * @type: 取消类型，2种结果：PTHREAD_CANCEL_DEFFERED，收到信号后继续运行至下一个取消点再退出
 *                          PTHREAD_CANCEL_ASYCHRONOUS，立即执行取消动作（退出）
 * @oldtype: oldtype如果不为NULL则存入原来的取消动作类型值。 
 * 
 * 成功返回0，失败返回-1
 */
int sys_thread_setcanceltype(int type, int *oldtype)
{
    task_t *cur = task_current;
    if (oldtype != NULL) {  /* 保存原来的类型 */
#ifdef DEBUG_PTHREAD
        printk(KERN_DEBUG "sys_thread_setcanceltype: pid=%d fetch oldtype.\n",
            task_current->pid);
#endif 
        if (cur->flags & THREAD_FLAG_CANCEL_ASYCHRONOUS) {
            *oldtype = PTHREAD_CANCEL_ASYCHRONOUS;  
        } else {
            *oldtype = PTHREAD_CANCEL_DEFFERED;
        }
    }
    if (type == PTHREAD_CANCEL_ASYCHRONOUS) {
        cur->flags |= THREAD_FLAG_CANCEL_ASYCHRONOUS;
#ifdef DEBUG_PTHREAD
        printk(KERN_DEBUG "sys_thread_setcanceltype: pid=%d set cancel asychronous.\n",
            task_current->pid);
#endif 
    } else if (type == PTHREAD_CANCEL_DEFFERED) {
        cur->flags &= ~THREAD_FLAG_CANCEL_ASYCHRONOUS;
#ifdef DEBUG_PTHREAD
        printk(KERN_DEBUG "sys_thread_setcanceltype: pid=%d set cancel deffered.\n",
            task_current->pid);
#endif 
    } else {
        return -1;
    }
    return 0;
}
