#include <xbook/schedule.h>
#include <xbook/process.h>
#include <xbook/pthread.h>
#include <xbook/safety.h>
#include <arch/interrupt.h>
#include <arch/task.h>
#include <errno.h>

void pthread_desc_init(pthread_desc_t *pthread)
{
    if (pthread != NULL) {
        atomic_set(&pthread->thread_count, 1);
    }
}

void pthread_desc_exit(pthread_desc_t *pthread)
{
    if (pthread != NULL)
        mem_free(pthread);
}

void pthread_entry(void *arg) 
{
    trap_frame_t *frame = TASK_GET_TRAP_FRAME(task_current);
    kernel_switch_to_user(frame);
}

int wait_one_hangging_thread(task_t *parent, pid_t pid, int *status)
{
    task_t *child, *next;
    list_for_each_owner_safe (child, next, &task_global_list, global_list) {
        if (child->pid == pid) {
            if (child->state == TASK_HANGING) {
                if (status != NULL)
                    *status = child->exit_status;
                proc_destroy(child, 1);
                return pid;
            }
        }
    }
    return -1;
}

task_t *pthread_start(task_func_t *func, void *arg, 
    pthread_attr_t *attr, void *thread_entry)
{
    task_t *parent = task_current;
    if (parent->pthread == NULL) {
        if (proc_pthread_init(parent))
            return NULL;
    }
    task_t *task = (task_t *) mem_alloc(TASK_KERN_STACK_SIZE);
    if (!task)
        return NULL;
    task_init(task, "pthread", TASK_PRIO_LEVEL_NORMAL);
    if (parent->tgid == parent->pid) {
        task->tgid = parent->pid;   /* 线程组id指向父进程的pid */
    } else {
        task->tgid = parent->tgid;   /* 线程组指向父进程的线程组 */
    }
    task->parent_pid = parent->pid;
    task->pthread = parent->pthread;
    task->vmm = parent->vmm;
    task->fileman = parent->fileman;
    exception_manager_init(&task->exception_manager);
    
    proc_trap_frame_init(task);
    task_stack_build(task, pthread_entry, arg);
    trap_frame_t *frame = TASK_GET_TRAP_FRAME(task);
    user_thread_frame_build(frame, arg, (void *)func, thread_entry, 
        (unsigned char *)attr->stackaddr + attr->stacksize);
    if (attr->detachstate == PTHREAD_CREATE_DETACHED) {
        task->flags |= THREAD_FLAG_DETACH;
    }
    
    unsigned long flags;
    interrupt_save_and_disable(flags);
    atomic_inc(&task->pthread->thread_count);
    if (atomic_get(&task->pthread->thread_count) > PTHREAD_MAX_NR) {
        atomic_dec(&task->pthread->thread_count);
        mem_free(task);
        interrupt_restore_state(flags);
        return NULL;
    }
    task_add_to_global_list(task);
    sched_queue_add_tail(sched_get_cur_unit(), task);
    interrupt_restore_state(flags);
    return task;
}

pid_t sys_thread_create(
    pthread_attr_t *attr,
    task_func_t *func,
    void *arg,
    void *thread_entry)
{
    if (!attr || !func || !thread_entry)
        return -EINVAL;

    if (mem_copy_from_user(NULL, attr, sizeof(pthread_attr_t)) < 0 ||
        safety_check_range(func, sizeof(task_func_t *)) < 0 ||
        safety_check_range(thread_entry, sizeof(void *)) < 0)
        return -EINVAL;

    task_t *task = pthread_start(func, arg, attr, thread_entry);
    if (task == NULL)
        return -ENOMEM;
    return task->pid;
}

void pthread_exit(void *status)
{
    unsigned long flags;
    interrupt_save_and_disable(flags);
    task_t *cur = task_current;
    atomic_dec(&cur->pthread->thread_count);   
    if (atomic_get(&cur->pthread->thread_count) == 0) {
        kprint(PRINT_DEBUG "pthread_exit: pid=%d no other threads, exit process!\n", cur->pid);
        sys_exit((int) status);
    }
    cur->exit_status = (int)status;
    task_do_cancel(cur);
    if (cur->flags & THREAD_FLAG_DETACH) {
        if (cur->flags &  THREAD_FLAG_JOINED) {
            task_t *parent = task_find_by_pid(cur->parent_pid);
            if (parent != NULL && parent->state == TASK_WAITING) {
                if (parent->tgid == cur->tgid) {
                    parent->flags &= ~THREAD_FLAG_JOINING;
                    task_unblock(parent);
                }
            }
        }
        cur->parent_pid = USER_INIT_PROC_ID;
    }
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

void sys_thread_exit(void *retval)
{
    pthread_exit(retval);
}

int sys_thread_detach(pthread_t thread) 
{
    task_t *task = task_find_by_pid(thread);
    if (task == NULL)
        return -1;
    kprint(PRINT_DEBUG "sys_thread_detach: thread=%s pid=%d tgid=%d ppid=%d set detach.\n",
        task->name, task->pid, task->tgid, task->parent_pid);
    task->flags |= THREAD_FLAG_DETACH;
    if (task->flags & THREAD_FLAG_JOINED) {
        task_t *parent = task_find_by_pid(task->parent_pid);
        if (parent != NULL && parent->state == TASK_WAITING) {
            if (parent->tgid == task->tgid) {
                kprint(PRINT_DEBUG "pthread_exit: pid=%d parent %s pid=%d joining, wakeup it.\n",
                    task->pid, parent->name, parent->pid);
                parent->flags &= ~THREAD_FLAG_JOINING;
                task_unblock(parent);
            }
        }
    }
    return 0;
}

int pthread_join(pthread_t thread, void **thread_return)
{
    task_t *waiter = task_current;
    unsigned long flags;
    interrupt_save_and_disable(flags);
    task_t *task, *find = NULL;
    list_for_each_owner (task, &task_global_list, global_list) {
        if (task->pid == thread && task->state != TASK_ZOMBIE) {
            find = task;
            break;
        }
    }
    if (find == NULL) {
        interrupt_restore_state(flags);
        return -1;
    }
    if (find->flags & THREAD_FLAG_DETACH) {
        interrupt_restore_state(flags);
        return -1;
    }
    if (find->flags & THREAD_FLAG_JOINED) {
        interrupt_restore_state(flags);
        return -1;
    }

    find->flags |= THREAD_FLAG_JOINED;
    find->parent_pid = waiter->pid;
    waiter->flags |= THREAD_FLAG_JOINING;
    int status;
    pid_t pid;
    do {
        status = 0;
        pid = wait_one_hangging_thread(waiter, find->pid, &status);
        if (pid == thread) {
            break;
        }
        if (!(waiter->flags & THREAD_FLAG_JOINING)) {
            break;
        }
        interrupt_restore_state(flags);
        task_block(TASK_WAITING);
        interrupt_save_and_disable(flags);
    } while (pid == -1);
    if (thread_return != NULL) {
        if (mem_copy_to_user(thread_return, &status, sizeof(void *)) < 0)
            return -1;
    }
    interrupt_restore_state(flags);
    return 0;
}

int sys_thread_join(pthread_t thread, void **thread_return)
{
    TASK_CHECK_THREAD_CANCELATION_POTINT(task_current);
    return pthread_join(thread, thread_return);
}

int sys_thread_cancel(pthread_t thread)
{
    task_t *task;
    unsigned long flags;
    interrupt_save_and_disable(flags);
    task = task_find_by_pid(thread);
    if (task == NULL) {
        interrupt_restore_state(flags);
        return -1;
    }
    task->flags |= THREAD_FLAG_CANCELED;
    if (task->flags & THREAD_FLAG_CANCEL_ASYCHRONOUS) {
        if (task == task_current) {
            interrupt_restore_state(flags);
            pthread_exit((void *) THREAD_FLAG_CANCEL_ASYCHRONOUS);
        } else {
            proc_close_one_thread(task);
        }
    }
    interrupt_restore_state(flags);
    return 0;
}

void sys_thread_testcancel(void)
{
    TASK_CHECK_THREAD_CANCELATION_POTINT(task_current);
}

/**
 * @state: 状态：PTHREAD_CANCEL_ENABLE（缺省）, 收到信号后设为CANCLED状态
 *              PTHREAD_CANCEL_DISABLE, 忽略CANCEL信号继续运行             
 * @oldstate: 原来的状态,old_state如果不为NULL则存入原来的Cancel状态以便恢复。 
 */
int sys_thread_setcancelstate(int state, int *oldstate)
{
    task_t *cur = task_current;
    if (oldstate != NULL) {
        if (cur->flags & THREAD_FLAG_CANCEL_DISABLE) {
            int _oldstate = PTHREAD_CANCEL_DISABLE;
            if (mem_copy_to_user(oldstate, &_oldstate, sizeof(int)) < 0)
                return -1;
        } else {
            int _oldstate = PTHREAD_CANCEL_ENABLE;
            if (mem_copy_to_user(oldstate, &_oldstate, sizeof(int)) < 0)
                return -1;
        }
    }
    if (state == PTHREAD_CANCEL_DISABLE) {
        cur->flags |= THREAD_FLAG_CANCEL_DISABLE;
    } else if (state == PTHREAD_CANCEL_ENABLE) {
        cur->flags &= ~THREAD_FLAG_CANCEL_DISABLE;
    } else {
        return -1;
    }
    return 0;
}

/**
 * @type: 取消类型，2种结果：PTHREAD_CANCEL_DEFFERED，收到信号后继续运行至下一个取消点再退出
 *                          PTHREAD_CANCEL_ASYCHRONOUS，立即执行取消动作（退出）
 * @oldtype: oldtype如果不为NULL则存入原来的取消动作类型值。 
 * 成功返回0，失败返回-1
 */
int sys_thread_setcanceltype(int type, int *oldtype)
{
    task_t *cur = task_current;
    if (oldtype != NULL) {
        if (cur->flags & THREAD_FLAG_CANCEL_ASYCHRONOUS) {
            int _oldtype = PTHREAD_CANCEL_ASYCHRONOUS;
            if (mem_copy_to_user(oldtype, &_oldtype, sizeof(int)) < 0)
                return -1;
        } else {
            int _oldtype = PTHREAD_CANCEL_DEFFERED;
            if (mem_copy_to_user(oldtype, &_oldtype, sizeof(int)) < 0)
                return -1;
        }
    }
    if (type == PTHREAD_CANCEL_ASYCHRONOUS) {
        cur->flags |= THREAD_FLAG_CANCEL_ASYCHRONOUS;
    } else if (type == PTHREAD_CANCEL_DEFFERED) {
        cur->flags &= ~THREAD_FLAG_CANCEL_ASYCHRONOUS;
    } else {
        return -1;
    }
    return 0;
}