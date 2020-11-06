#include <arch/interrupt.h>
#include <xbook/schedule.h>
#include <xbook/debug.h>
#include <xbook/task.h>
#include <xbook/process.h>
#include <xbook/pthread.h>
#include <xbook/trigger.h>
#include <sys/wait.h>

/*
僵尸进程：当进程P调用exit时，其父进程没有wait，那么它就变成一个僵尸进程。
        只占用PCB，不占用内存其它资源。
        处理方法：父进程退出时，会查看是否有僵尸进程，并且将它们回收。
孤儿进场：当进程P还未调用exit时，其父进程就退出了，那么它就变成一个孤儿进程。
        不过它可以交给init进程收养。
        处理方法：将进程P过继给INIT进程收养。
*/

int wait_any_hangging_child(task_t *parent, int *status)
{
    task_t *child, *next;
    list_for_each_owner_safe (child, next, &task_global_list, global_list) {
        if (child->parent_pid == parent->pid) {
            if (child->state == TASK_HANGING) {
                pid_t child_pid = child->pid;
                if (status != NULL)
                    *status = child->exit_status;
                if (TASK_IS_SINGAL_THREAD(child)) {
                    proc_destroy(child, 0);
                } else {
                    proc_destroy(child, 1);
                }     
                return child_pid;
            }
        }
    }
    return -1;
}

int wait_one_hangging_child(task_t *parent, pid_t pid, int *status)
{
    task_t *child, *next;
    list_for_each_owner_safe (child, next, &task_global_list, global_list) {
        if (child->pid == pid) { 
            if (child->state == TASK_HANGING) {
                if (status != NULL)
                    *status = child->exit_status;
                if (TASK_IS_SINGAL_THREAD(child)) {
                    proc_destroy(child, 0);
                } else {
                    proc_destroy(child, 1);
                }
                return pid;
            }
        }
    }
    return -1;
}

int deal_zombie_child(task_t *parent)
{
    int zombies = 0;
    int zombie = -1;
    task_t *child, *next;
    list_for_each_owner_safe (child, next, &task_global_list, global_list) {
        if (child->parent_pid == parent->pid) {
            if (child->state == TASK_ZOMBIE) {
                if (zombie == -1) {
                    zombie = child->pid;
                }
                if (TASK_IS_SINGAL_THREAD(child)) {
                    proc_destroy(child, 0);
                } else {
                    proc_destroy(child, 1);
                }
                zombies++;
            }
        }
    }
    return zombie; /* 如果没有僵尸进程就返回-1，有则返回第一个僵尸进程的pid */
}

int find_child_proc(task_t *parent)
{
    int children = 0;
    task_t *child;
    list_for_each_owner (child, &task_global_list, global_list) {
        if (child->parent_pid == parent->pid && TASK_IS_SINGAL_THREAD(child)) {
            children++;
        }
    }
    return children;
}

void adopt_children_to_init(task_t *parent)
{
    task_t *child;
    list_for_each_owner (child, &task_global_list, global_list) {
        if (child->parent_pid == parent->pid) {
            child->parent_pid = USER_INIT_PROC_ID;
        }
    }
}

void close_one_thread(task_t *thread)
{
    if (thread->state == TASK_READY) {
        list_del_init(&thread->list);
    }
    if (thread->state != TASK_HANGING && thread->state != TASK_ZOMBIE) {
        thread_release_resource(thread);
    }
    proc_destroy(thread, 1);
}

void close_other_threads(task_t *thread)
{
    task_t *borther, *next;
    list_for_each_owner_safe (borther, next, &task_global_list, global_list) {
        if (TASK_IN_SAME_THREAD_GROUP(thread, borther)) {
            if (thread->pid != borther->pid) {
                close_one_thread(borther);
            }
        }
    }
    if (thread->pthread) {
        atomic_set(&thread->pthread->thread_count, 0);
    }
}

/**
 * @pid: 指定的进程id:
 *      为-1时，等待任意子进程，>0时等待指定的子进程
 * @status: 存放子进程退出时的状态 
 * @options: 选项参数：
 *          WNOHANG: 如果没有子进程退出，就不阻塞等待，返回0
 * 等待流程：
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
    task_t *parent = task_current;
    TASK_CHECK_THREAD_CANCELATION_POTINT(parent);
    pid_t child_pid;
    unsigned long flags;
    while (1) {
        interrupt_save_and_disable(flags);
        if (pid == -1) { 
            if ((child_pid = wait_any_hangging_child(parent, status)) >= 0) {
                interrupt_restore_state(flags);
                return child_pid;
            }
        } else {
            
            if ((child_pid = wait_one_hangging_child(parent, pid, status)) >= 0) {
                interrupt_restore_state(flags);
                return child_pid;
            }
        }
        if ((child_pid = deal_zombie_child(parent)) > 0) {
            interrupt_restore_state(flags);
            return child_pid;
        }
        if (!find_child_proc(parent)) {
            interrupt_restore_state(flags);
            return -1;
        }
        if (options & WNOHANG) {
            interrupt_restore_state(flags);
            return 0;
        }
        interrupt_restore_state(flags);
        task_block(TASK_WAITING);
        if (trigismember(&task_current->triggers->set, TRIGHSOFT) ||
            trigismember(&task_current->triggers->set, TRIGLSOFT) ||
            trigismember(&task_current->triggers->set, TRIGSYS)) {
            break;
        }
    }
    return -1;
}

/**
 * @status: 退出状态
 * 退出流程：
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
    interrupt_save_and_disable(flags);
    task_t *cur = task_current;
    cur->exit_status = status;
    close_other_threads(cur);
    deal_zombie_child(cur);
    adopt_children_to_init(cur);
    proc_release(cur);
    task_t *parent = task_find_by_pid(cur->parent_pid); 
    if (parent) {
        if (parent->state == TASK_WAITING) {
            interrupt_restore_state(flags);
            task_unblock(parent);
            task_block(TASK_HANGING);
        } else {
            interrupt_restore_state(flags);
            task_block(TASK_ZOMBIE);
        }
    } else {
        interrupt_restore_state(flags);
        task_block(TASK_ZOMBIE); 
    }
}

void kern_thread_exit(int status)
{
    unsigned long flags;
    interrupt_save_and_disable(flags);

    task_t *cur = task_current;
    cur->exit_status = status;
    thread_release_resource(cur);
    cur->parent_pid = USER_INIT_PROC_ID;
    task_t *parent = task_find_by_pid(cur->parent_pid); 
    if (parent) {
        if (parent->state == TASK_WAITING) {
            interrupt_restore_state(flags);
            task_unblock(parent);
            task_block(TASK_HANGING);
        } else {
            interrupt_restore_state(flags);
            task_block(TASK_ZOMBIE);
        }
    } else {
        interrupt_restore_state(flags);
        task_block(TASK_ZOMBIE); 
    }
}