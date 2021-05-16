#include <xbook/mutexlock.h>
#include <xbook/schedule.h>

void mutex_lock(mutexlock_t *mutex)
{
    while (1) {
        if (!spin_try_lock(&mutex->wait_lock)) {
            return;
        }
        #if !defined(MUTEX_LOCK_TINY)
        mutex->waiters++;
        list_add_tail(&task_current->list, &mutex->wait_list);
        TASK_ENTER_WAITLIST(task_current);
        task_block(TASK_BLOCKED);
        #endif
    };
}

void mutex_unlock(mutexlock_t *mutex)
{
    unsigned long flags;
    interrupt_save_and_disable(flags);
    spin_unlock(&mutex->wait_lock);
    #if !defined(MUTEX_LOCK_TINY)
    if (mutex->waiters < 1) {
        interrupt_restore_state(flags);
        return;
    }
    task_t *task = list_first_owner_or_null(&mutex->wait_list, task_t, list);
    if (task) {
        list_del_init(&task->list);
        TASK_LEAVE_WAITLIST(task_current);
        mutex->waiters--;
        task_wakeup(task);
    }
    #endif
    interrupt_restore_state(flags);
}
