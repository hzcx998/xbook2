#include <arch/interrupt.h>
#include <xbook/schedule.h>
#include <xbook/debug.h>
#include <xbook/task.h>
#include <xbook/process.h>
#include <xbook/pthread.h>
#include <sys/wait.h>

// #define DEBUG_WAIT_EXIT

/*
僵尸进程：当进程P调用exit时，其父进程没有wait，那么它就变成一个僵尸进程。
        只占用PCB，不占用内存其它资源。
        处理方法：父进程退出时，会查看是否有僵尸进程，并且将它们回收。
孤儿进场：当进程P还未调用exit时，其父进程就退出了，那么它就变成一个孤儿进程。
        不过它可以交给init进程收养。
        处理方法：将进程P过继给INIT进程收养。
*/

/**
 * wait_any_hangging_child - 处理一个挂起子进程
 * 
 */
int wait_any_hangging_child(task_t *parent, int *status)
{
    task_t *child, *next;
    /* 可能删除列表元素，需要用safe */
    list_for_each_owner_safe (child, next, &task_global_list, global_list) {
        if (child->parent_pid == parent->pid) { /* find a child process */
            if (child->state == TASK_HANGING) { /* child is hanging, destroy it  */
                pid_t child_pid = child->pid;
#ifdef DEBUG_WAIT_EXIT
                printk(KERN_NOTICE "wait_any_hangging_child: pid=%d find a hanging proc %d \n",
                    parent->pid, child_pid);
#endif              
                /* 状态是可以为空的，不为空才写入子进程退出状态 */
                if (status != NULL)
                    *status = child->exit_status;
                /* 子进程或者子线程 */
                if (IN_SINGAL_THREAD(child)) {
#ifdef DEBUG_WAIT_EXIT
                    printk(KERN_NOTICE "wait_any_hangging_child: process.\n");
                    if (child->pthread)
                        printk(KERN_NOTICE "wait_any_hangging_child: thread count %d.\n",
                            atomic_get(&child->pthread->thread_count));
#endif                    
                    /* 销毁子进程的PCB */
                    proc_destroy(child, 0);
                } else {
#ifdef DEBUG_WAIT_EXIT
                    printk(KERN_NOTICE "wait_any_hangging_child: thread.\n");
#endif       
                    /* 销毁子线程的PCB */
                    proc_destroy(child, 1);
                }     
                return child_pid;
            }
        }
    }
    return -1;
}

/**
 * wait_one_hangging_child - 处理一个挂起子进程
 * 
 */
int wait_one_hangging_child(task_t *parent, pid_t pid, int *status)
{
    task_t *child, *next;
    /* 可能删除列表元素，需要用safe */
    list_for_each_owner_safe (child, next, &task_global_list, global_list) {
        if (child->pid == pid) { /* find a child process we ordered */
            if (child->state == TASK_HANGING) { /* child is hanging, destroy it  */
#ifdef DEBUG_WAIT_EXIT
                printk(KERN_NOTICE "wait_one_hangging_child: pid=%d find a hanging task %d \n",
                    parent->pid, pid);
#endif              
                /* 状态是可以为空的，不为空才写入子进程退出状态 */
                if (status != NULL)
                    *status = child->exit_status;

                /* 子进程或者子线程 */
                if (IN_SINGAL_THREAD(child)) {
#ifdef DEBUG_WAIT_EXIT
                    printk(KERN_NOTICE "wait_one_hangging_child: process.\n");
                    if (child->pthread)
                        printk(KERN_NOTICE "wait_one_hangging_child: thread count %d.\n",
                            atomic_get(&child->pthread->thread_count));
                    
#endif                    
                    /* 销毁子进程的PCB */
                    proc_destroy(child, 0);
                } else {
#ifdef DEBUG_WAIT_EXIT
                    printk(KERN_NOTICE "wait_one_hangging_child: thread.\n");
#endif                    
                    /* 销毁子线程的PCB */
                    proc_destroy(child, 1);
                }
                return pid;
            }
        }
    }
    return -1;
}

/**
 * deal_zombie_child - 处理僵尸子进程
 * 
 */
int deal_zombie_child(task_t *parent)
{
    int zombies = 0;        /* 僵尸进程数 */
    int zombie = -1;        /* 第一个僵尸进程 */
    task_t *child, *next;
    list_for_each_owner_safe (child, next, &task_global_list, global_list) {
        if (child->parent_pid == parent->pid) {
            if (child->state == TASK_ZOMBIE) { /* child is zombie, destroy it  */
                if (zombie == -1) {
                    zombie = child->pid;
                }
                
#ifdef DEBUG_WAIT_EXIT
                printk(KERN_NOTICE "deal_zombie_child: pid=%d find a zombie child %d \n", parent->pid, child->pid);
#endif
                /* 子进程或者子线程 */
                if (IN_SINGAL_THREAD(child)) {
#ifdef DEBUG_WAIT_EXIT
                    printk(KERN_NOTICE "deal_zombie_child: process.\n");
                    if (child->pthread)
                        printk(KERN_NOTICE "deal_zombie_child: thread count %d.\n",
                            atomic_get(&child->pthread->thread_count));
#endif                    
                    /* 销毁子进程的PCB */
                    proc_destroy(child, 0);
                } else {
#ifdef DEBUG_WAIT_EXIT
                    printk(KERN_NOTICE "deal_zombie_child: thread.\n");
#endif                    
                    /* 销毁子线程的PCB */
                    proc_destroy(child, 1);
                }
                zombies++;

            }
        }
    }
    return zombie;  /* 如果没有僵尸进程就返回-1，有则返回第一个僵尸进程的pid */
}

/**
 * find_child_proc - 查找子进程
 * @parent: 父进程
 * 
 * 返回子进程数，没有则返回0
*/
int find_child_proc(task_t *parent)
{
    int children = 0;
    task_t *child;
    list_for_each_owner (child, &task_global_list, global_list) {
        /* 必须是单线程才可以 */
        if (child->parent_pid == parent->pid && IN_SINGAL_THREAD(child)) {
            children++;
        }
    }
    return children; /* return child proc number */
}

/**
 * adopt_children_to_init - 过继子进程给INIT进程
 * @parent: 父进程
 * 
 * 返回子进程数，没有则返回0
*/
void adopt_children_to_init(task_t *parent)
{
    task_t *child;
    list_for_each_owner (child, &task_global_list, global_list) {
        if (child->parent_pid == parent->pid) { /* find a child process */
            child->parent_pid = INIT_PROC_PID;
        }
    }
}

/**
 * close_one_thread - 强制关闭一个其它线程
 * @thread: 线程
 * 
 * 需要在任务安全环境中调用，即过程中不能产生中断
 */
void close_one_thread(task_t *thread)
{
#ifdef DEBUG_WAIT_EXIT
    printk(KERN_DEBUG "close_one_thread: task=%s pid=%d tgid=%d ppid=%d state=%d\n",
        thread->name, thread->pid, thread->tgid, thread->parent_pid, thread->state);
#endif
    /* 如果线程处于就绪状态，那么就从就绪队列删除 */
    if (thread->state == TASK_READY) {
        /* 从优先级队列移除 */
        list_del_init(&thread->list);
        /* 对应的优先级队列任务数量-1 */
        // thread->prio_queue->length--;
        // thread->prio_queue = NULL;
#ifdef DEBUG_WAIT_EXIT
        printk(KERN_DEBUG "close_one_thread: pid=%d remove from ready list.\n",
            thread->pid);
#endif
    }
    /* 挂起和僵尸态已经释放了线程资源 */
    if (thread->state != TASK_HANGING && thread->state != TASK_ZOMBIE) {
        thread_release_resource(thread);  /* 释放线程资源 */
    }
    /* 销毁线程 */
    proc_destroy(thread, 1);

}

/**
 * close_other_threads - 关闭当前线程组中的其它线程
 * @thread: 当前要关闭的线程
 * 
 */
void close_other_threads(task_t *thread)
{
    /* 查找所有的兄弟线程 */
    task_t *borther, *next;
    list_for_each_owner_safe (borther, next, &task_global_list, global_list) {
        /* 查找一个兄弟线程，位于同一个线程组，但不是自己 */
        if (IN_SAME_THREAD_GROUP(thread, borther)) {
            if (thread->pid != borther->pid) {
                close_one_thread(borther); /* 销毁兄弟 */
            }
        }
    }
    /* 有线程，就把线程数设为0，表示目前是一个单进程，退出的时候就可以判断 */
    if (thread->pthread) {
        atomic_set(&thread->pthread->thread_count, 0);
    }
}

/**
 * sys_waitpid - 进程等待
 * @pid: 指定的进程id:
 *      为-1时，等待任意子进程，>0时等待指定的子进程
 * @status: 存放子进程退出时的状态 
 * @options: 选项参数：
 *          WNOHANG: 如果没有子进程退出，就不阻塞等待，返回0
 * 
 * 等待流程：
 * 
 * 查看是否有挂起的进程->
 *      有则销毁挂起的进程，并返回进程pid，END
 * 判断是否有子进程->
 *      没有发现子进程，就返回-1，END
 * 继续处于WAITING状态，直到有子进程退出
 * 
 * 返回子进程的pid，无则返回-1
 */
pid_t sys_waitpid(pid_t pid, int *status, int options)
{
    
    task_t *parent = current_task;  /* 当前进程是父进程 */
    CHECK_THREAD_CANCELATION_POTINT(parent);
    pid_t child_pid;
    unsigned long flags;
    
    while (1) {
#ifdef DEBUG_WAIT_EXIT
        printk(KERN_DEBUG "sys_wait: name=%s pid=%d wait child.\n", parent->name, parent->pid);
#endif    
        save_intr(flags);
        if (pid == -1) {    /* 任意子进程 */            
            /* 先处理挂起的任务 */
            if ((child_pid = wait_any_hangging_child(parent, status)) >= 0) {
#ifdef DEBUG_WAIT_EXIT
                printk(KERN_DEBUG "sys_wait: pid=%d handle a hangging child %d.\n",
                    parent->pid, child_pid);
#endif    
                restore_intr(flags);
                return child_pid;
            }
        } else {    /* 指定子进程 */
            /* 先处理挂起的任务 */
            if ((child_pid = wait_one_hangging_child(parent, pid, status)) >= 0) {
#ifdef DEBUG_WAIT_EXIT
                printk(KERN_DEBUG "sys_wait: pid=%d handle a hangging child %d.\n",
                    parent->pid, child_pid);
#endif    
                restore_intr(flags);
                return child_pid;
            }
        }
        
        /* 处理zombie子进程 */
        if ((child_pid = deal_zombie_child(parent)) > 0) {
            restore_intr(flags);
            //printk("[task]: parent %d deal zombie child %d\n", parent->pid, child_pid);
            return child_pid;
        }

#ifdef DEBUG_WAIT_EXIT
        printk(KERN_DEBUG "sys_wait: check no wait!\n");
#endif
       
        /* 查看是否有其它子进程 */
        if (!find_child_proc(parent)) {
#ifdef DEBUG_WAIT_EXIT
            printk(KERN_DEBUG "sys_wait: no children!\n");
#endif    
            restore_intr(flags);
            return -1; /* no child, return -1 */
        }

        /* 现在肯定没有子进程处于退出状态 */
        if (options & WNOHANG) {    /* 有不挂起等待标志，就直接返回 */
            restore_intr(flags);
            return 0;   /* 不阻塞等待，返回0 */
        }

#ifdef DEBUG_WAIT_EXIT
        printk(KERN_DEBUG "sys_wait: pid=%d no child exit, waiting...\n", parent->pid);
#endif
        restore_intr(flags);
        /* WATING for children to exit */
        task_block(TASK_WAITING);
        //printk(KERN_DEBUG "proc wait: child unblocked me.\n");
    }
}

/**
 * sys_exit - 进程退出
 * @status: 退出状态
 * 
 * 退出流程：
 * 
 * 设置退出状态->查看是否有父进程->
 *      无父进程->PANIC，END
 * 查看子进程是否有僵尸进程，有就把僵尸进程释放掉，解决僵尸状态->把其它子进程过继给INIT进程，解决孤儿进程问题。
 * 释放占用资源->查看父进程是否处于等待状态->
 *      是等待状态->解除对父进程的阻塞。->自己变成挂起状态，等待父进程来销毁。END
 *      不是等待状态->把自己设置成僵尸状态，等待父进程退出。END
 */
void sys_exit(int status)
{
    unsigned long flags;
    save_intr(flags);

    task_t *cur = current_task;  /* 当前进程是父进程 */

    cur->exit_status = status;
    
    /*if (cur->parent_pid == -1 || parent == NULL)
        panic("sys_exit: proc %s PID=%d exit with parent pid -1!\n", cur->name, cur->pid);
    */
#ifdef DEBUG_WAIT_EXIT
    printk(KERN_DEBUG "sys_exit: name=%s pid=%d ppid=%d prio=%d status=%d\n",
        cur->name, cur->pid, cur->parent_pid, cur->priority, cur->exit_status);
#endif    
    
    /* 关闭其它线程 */
    close_other_threads(cur);
#ifdef DEBUG_WAIT_EXIT
    if (cur->pthread)
        printk(KERN_DEBUG "sys_exit: thread count %d\n", atomic_get(&cur->pthread->thread_count));
#endif    

    /* 处理zombie子进程或子线程 */
    deal_zombie_child(cur);

    /* 过继子进程给init进程，只剩运行中的子进程还未处理，需要过继 */
    adopt_children_to_init(cur);
    
    /* 释放占用的资源 */
    proc_release(cur); /* 释放资源 */
#ifdef DEBUG_WAIT_EXIT
    printk(KERN_DEBUG "sys_exit: pid=%d release all resources done.\n", cur->pid);
#endif    
    task_t *parent = find_task_by_pid(cur->parent_pid); 
    
    if (parent) {
        /* 查看父进程状态 */
        if (parent->state == TASK_WAITING) {
            restore_intr(flags);
#ifdef DEBUG_WAIT_EXIT
            printk(KERN_DEBUG "sys_exit: pid=%d parent %d waiting...\n", 
                cur->pid, parent->pid);
#endif    

            //printk("parent waiting...\n");
            task_unblock(parent); /* 唤醒父进程 */
            task_block(TASK_HANGING);   /* 把自己挂起 */
        } else { /* 父进程没有 */
            restore_intr(flags);
#ifdef DEBUG_WAIT_EXIT
            printk(KERN_DEBUG "sys_exit: pid=%d parent %d not waiting, zombie!\n",
                cur->pid, parent->pid);
#endif    
            //printk("parent not waiting, zombie!\n");
            task_block(TASK_ZOMBIE);   /* 变成僵尸进程 */
        }
    } else {
        /* 没有父进程，变成不可被收养的孤儿+僵尸进程 */
#ifdef DEBUG_WAIT_EXIT
            printk(KERN_DEBUG "sys_exit: pid=%d no parent! zombie!\n", cur->pid);
#endif    
        /* 可以赖皮，认INIT进程为干爸爸，那么就有父进程了，就可以被回收。 嘻嘻 */
        //printk("no parent!\n");
        restore_intr(flags);
        task_block(TASK_ZOMBIE); 
    }
}

void kthread_exit(int status)
{
    unsigned long flags;
    save_intr(flags);

    task_t *cur = current_task;  /* 当前进程是父进程 */
    cur->exit_status = status;

    /* 释放内核资源 */
    thread_release_resource(cur);
    
    /* 内核线程没有实际的父进程，因此把自己过继给init进程 */
    cur->parent_pid = INIT_PROC_PID;

    task_t *parent = find_task_by_pid(cur->parent_pid); 
    if (parent) {
        /* 查看父进程状态 */
        if (parent->state == TASK_WAITING) {
            restore_intr(flags);
#ifdef DEBUG_WAIT_EXIT
            printk(KERN_DEBUG "sys_exit: pid=%d parent %d waiting...\n", cur->pid, parent->pid);
#endif    

            //printk("parent waiting...\n");
            task_unblock(parent); /* 唤醒父进程 */
            task_block(TASK_HANGING);   /* 把自己挂起 */
        } else { /* 父进程没有 */
            restore_intr(flags);
#ifdef DEBUG_WAIT_EXIT
            printk(KERN_DEBUG "sys_exit: pid=%d parent %d not waiting, zombie!\n", cur->pid, parent->pid);
#endif    
            //printk("parent not waiting, zombie!\n");
            task_block(TASK_ZOMBIE);   /* 变成僵尸进程 */
        }
    } else {
        /* 没有父进程，变成不可被收养的孤儿+僵尸进程 */
#ifdef DEBUG_WAIT_EXIT
            printk(KERN_DEBUG "sys_exit: pid=%d no parent! zombie!\n", cur->pid);
#endif    
        //printk("no parent!\n");
        restore_intr(flags);
        task_block(TASK_ZOMBIE); 
    }
}
