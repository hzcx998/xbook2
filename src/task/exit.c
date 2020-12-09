#include <arch/interrupt.h>
#include <xbook/schedule.h>
#include <xbook/debug.h>
#include <xbook/task.h>
#include <xbook/process.h>
#include <xbook/pthread.h>
#include <xbook/fd.h>

static void adopt_children_to_init(task_t *parent)
{
    task_t *child;
    list_for_each_owner (child, &task_global_list, global_list) {
        if (child->parent_pid == parent->pid) {
            child->parent_pid = USER_INIT_PROC_ID;
        }
    }
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
    task_exit_hook(cur);
    proc_close_other_threads(cur);
    proc_deal_zombie_child(cur);
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
