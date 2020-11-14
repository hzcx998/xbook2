#include <sys/wait.h>
#include <xbook/task.h>
#include <xbook/schedule.h>
#include <xbook/process.h>
#include <xbook/safety.h>

/*
僵尸进程：当进程P调用exit时，其父进程没有wait，那么它就变成一个僵尸进程。
        只占用PCB，不占用内存其它资源。
        处理方法：父进程退出时，会查看是否有僵尸进程，并且将它们回收。
孤儿进场：当进程P还未调用exit时，其父进程就退出了，那么它就变成一个孤儿进程。
        不过它可以交给init进程收养。
        处理方法：将进程P过继给INIT进程收养。
*/
static int wait_any_hangging_child(task_t *parent, int *status)
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

static int wait_one_hangging_child(task_t *parent, pid_t pid, int *status)
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
    if (status && safety_check_range(status, sizeof(int *)) < 0)
        return -1;
    int wait_status = 0;
    task_t *parent = task_current;
    TASK_CHECK_THREAD_CANCELATION_POTINT(parent);
    pid_t child_pid;
    unsigned long flags;
    while (1) {
        interrupt_save_and_disable(flags);
        if (pid == -1) { 
            if ((child_pid = wait_any_hangging_child(parent, &wait_status)) >= 0) {
                interrupt_restore_state(flags);
                if (status)
                    mem_copy_to_user(status, &wait_status, sizeof(int *));
                return child_pid;
            }
        } else {
            
            if ((child_pid = wait_one_hangging_child(parent, pid, &wait_status)) >= 0) {
                interrupt_restore_state(flags);
                if (status)
                    mem_copy_to_user(status, &wait_status, sizeof(int *));
                return child_pid;
            }
        }
        if ((child_pid = proc_deal_zombie_child(parent)) > 0) {
            interrupt_restore_state(flags);
            return child_pid;
        }
        if (!task_count_children(parent)) {
            interrupt_restore_state(flags);
            return -1;
        }
        if (options & WNOHANG) {
            interrupt_restore_state(flags);
            return 0;
        }
        interrupt_restore_state(flags);
        task_block(TASK_WAITING);
        if (exception_cause_exit(&parent->exception_manager)) {
            break;
        }
    }
    return -1;
}