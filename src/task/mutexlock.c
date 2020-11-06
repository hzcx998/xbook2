#include <xbook/mutexlock.h>
#include <xbook/schedule.h>

void mutex_lock(mutexlock_t *mutex)
{
    while (1) {
        if (!spin_try_lock(&mutex->wait_lock)) {
            return;
        }
        mutex->waiters++;
        list_add_tail(&task_current->list, &mutex->wait_list);
        task_block(TASK_BLOCKED);
    };
}

void mutex_unlock(mutexlock_t *mutex)
{
    unsigned long flags;
    interrupt_save_and_disable(flags);
    spin_unlock(&mutex->wait_lock);
    if (mutex->waiters < 1) {
        interrupt_restore_state(flags);
        return;
    }
    task_t *task = list_first_owner(&mutex->wait_list, task_t, list);
    list_del_init(&task->list);
    mutex->waiters--;
    task_wakeup(task);
    interrupt_restore_state(flags);
}
