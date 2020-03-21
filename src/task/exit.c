#include <arch/interrupt.h>
#include <xbook/task.h>

extern list_t task_global_list;

/**
 * notify_parent - 提醒父进程
 */
static int notify_parent(int parent_pid)
{
    /*  a.如果父进程处于休眠中，在等待子进程唤醒 
        b.如果父进程在等待我唤醒之前就远去，就会把我过继给init，
    所以这里也是没有问题的 
    */
    
    int ret = -1;   /* 默认是没有父进程的 */

    task_t *parent;

    /* 在全局任务链表中查找子进程的父进程 */
    list_for_each_owner(parent, &task_global_list, global_list) {
        /* 如果进程的父pid和当前进程的pid一样，并且处于等待状态
        就说明这个进程就是当前进程的子进程 */
        if (parent->pid == parent_pid && parent->state == TASK_WAITING) {
            /*printk("my parent %s pid %d status %d is waitting for me, wake up him!\n",
                    parent->name, parent->pid, parent->status);
             */
            
            /* 向父进程发送信号 */
            //SysKill(parent->pid, SIGCHLD);
            //printk("send SIGCHLD to parent %s-%d\n", parent->name, parent->pid);
            /* 将父进程唤醒 */
            task_unblock(parent);
            
            ret = 0;    /* 有父进程在等待 */
            /* 唤醒后就退出查询，因为只有1个父亲，不能有多个父亲吧（偷笑） */
            break;
        }
    }
    return ret;
}

/**
 * adope_self - 把自己过继给init进程
 */
static void adope_self(task_t *cur)
{
    /* 父进程id */
    cur->parent_pid = 1;
    /* 过继给init之后还要提醒一下init才可以 */
    notify_parent(cur->parent_pid);
}

/**
 * kthread_exit - 关闭内核线程
 * @task: 要关闭的线程
 */
void kthread_exit(task_t *task)
{
    if (!task)
        return;
    
    /* 过继给init进程（pid为1） */
    adope_self(task);
    
    /* 操作链表时关闭中断，结束后恢复之前状态 */
    unsigned long flags;
    save_intr(flags);

    /* 如果在就绪队列中，就从就绪队列中删除 */
    if (is_task_in_priority_queue(task)) {
        list_del_init(&task->list);
    }
    restore_intr(flags);

    /* 调度出去，僵尸状态，等待父进程收尸 */
    task_block(TASK_ZOMBIE);
}
