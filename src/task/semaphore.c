#include <xbook/semaphore.h>
#include <xbook/task.h>
#include <xbook/schedule.h>

semaphore_t *semaphore_alloc(int value)
{
    semaphore_t *sema = mem_alloc(sizeof(semaphore_t));
    if (!sema)
        return NULL;
    semaphore_init(sema, value);
    return sema;
}

int semaphore_free(semaphore_t *sema)
{
    if (!sema)
        return -1;
    semaphore_destroy(sema);
    mem_free(sema);
    return 0;
}

void __semaphore_down(semaphore_t *sema, unsigned long iflags)
{
	list_add_tail(&task_current->list, &sema->waiter.wait_list);
    TASK_ENTER_WAITLIST(task_current);
    interrupt_restore_state(iflags);
	task_block(TASK_BLOCKED);
}

void __semaphore_up(semaphore_t *sema, unsigned long iflags)
{
	task_t *waiter = list_first_owner_or_null(&sema->waiter.wait_list, task_t, list);
	if (waiter) {
        list_del(&waiter->list);
        TASK_LEAVE_WAITLIST(waiter);
        waiter->state = TASK_READY;
        sched_queue_add_head(sched_get_cur_unit(), waiter);
    }
    interrupt_restore_state(iflags);
}
