#include <arch/interrupt.h>
#include <xbook/schedule.h>
#include <xbook/debug.h>
#include <xbook/task.h>
#include <xbook/process.h>

/*
僵尸进程：当进程P调用exit时，其父进程没有wait，那么它就变成一个僵尸进程。
        只占用PCB，不占用内存其它资源。
        处理方法：父进程退出时，会查看是否有僵尸进程，并且将它们回收。
孤儿进场：当进程P还未调用exit时，其父进程就退出了，那么它就变成一个孤儿进程。
        不过它可以交给init进程收养。
        处理方法：将进程P过继给INIT进程收养。
*/

/**
 * wait_one_hangging_child - 处理一个挂起子进程
 * 
*/
int wait_one_hangging_child(task_t *parent, int *status)
{
    task_t *child;
    list_for_each_owner (child, &task_global_list, global_list) {
        if (child->parent_pid == parent->pid) { /* find a child process */
            if (child->state == TASK_HANGING) { /* child is hanging, destroy it  */
                *status = child->exit_status;
                pid_t child_pid = child->pid;
                /* 销毁子进程的PCB */
                task_free(child);
                printk(KERN_NOTICE "find a hanging proc %d \n", child_pid);
                return child_pid;
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
    int zombie = 0;
    task_t *child, *next;
    list_for_each_owner_safe (child, next, &task_global_list, global_list) {
        if (child->parent_pid == parent->pid) { /* find a child process */
            if (child->state == TASK_ZOMBIE) { /* child is zombie, destroy it  */
                /* 销毁子进程的PCB */
                task_free(child);
                zombie++;
                
                printk(KERN_NOTICE "find a zombie proc %d \n", child->pid);
            }
        }
    }
    return zombie;
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
        if (child->parent_pid == parent->pid) { /* find a child process */
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
 * proc_wait - 进程等待 
 * @status: 存放子进程退出时的状态 
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
pid_t proc_wait(int *status)
{
    task_t *parent = current_task;  /* 当前进程是父进程 */
    pid_t child_pid;
    while (1) {
        printk(KERN_DEBUG "proc wait: name=%s pid=%d wait child.\n", parent->name, parent->pid);
        /* 先处理挂起的任务 */
        if ((child_pid = wait_one_hangging_child(parent, status)) >= 0) {
            printk("proc wait exit\n");
            return child_pid;
        }
        printk(KERN_DEBUG "proc wait: find proc.\n");
        /* 查看是否有子进程 */
        if (!find_child_proc(parent)) 
            return -1; /* no child, return -1 */
        
        printk(KERN_DEBUG "proc wait: no child exit, waiting...\n");
        /* WATING for children to exit */
        task_block(TASK_WAITING);
        printk(KERN_DEBUG "proc wait: child unblocked me.\n");
    }
}

/**
 * proc_exit - 进程退出
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
void proc_exit(int status)
{
    task_t *cur = current_task;  /* 当前进程是父进程 */
    cur->exit_status = status;
    task_t *parent = find_task_by_pid(cur->parent_pid); 
    
    if (cur->parent_pid == -1 || parent == NULL)
        panic("proc_exit: proc %s exit with parent pid -1!\n", cur->name);
    printk(KERN_DEBUG "proc exit name=%s pid=%d ppid=%d prio=%d\n", cur->name, cur->pid, cur->parent_pid, cur->priority);
    /* 处理zombie子进程 */
    deal_zombie_child(cur);
    /* 过继子进程给init进程 */
    adopt_children_to_init(cur);

    /* 释放占用的资源 */
    proc_vmm_exit(cur); /* 内存资源退出 */
    
    /* 查看父进程状态 */
    if (parent->state == TASK_WAITING) {
        printk("parent waiting...\n");
        task_unblock(parent); /* 唤醒父进程 */
        task_block(TASK_HANGING);   /* 把自己挂起 */
    } else { /* 没有找到父进程 */
        printk("parent not waiting, zombie!\n");
        task_block(TASK_ZOMBIE);   /* 变成僵尸进程 */
    }
}
