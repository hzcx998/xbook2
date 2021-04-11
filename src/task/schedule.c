#include <xbook/schedule.h>
#include <xbook/task.h>
#include <xbook/clock.h>
#include <assert.h>
#include <xbook/debug.h>
#include <arch/interrupt.h>
#include <arch/task.h>

#define DEBUG_SCHED 0

scheduler_t scheduler;

const uint8_t sched_priority_levels[TASK_PRIO_LEVEL_MAX] = {1, 0, 1, 2, 3};

/*
TODO：优化动态优先级：当一个优先级高的线程长期获得优运行时，可以适当调低优先级。
当一个低先级高的线程长期没获得运行时，可以适当调提高优先级。
当前采取的是高优先级执行固定时间后就降低，不太好，没体现优先级的优势。
*/
uint8_t sched_calc_base_priority(uint32_t level)
{
    if (level >= TASK_PRIO_LEVEL_MAX)
        level = 0;
    return sched_priority_levels[level];
}

uint8_t sched_calc_new_priority(task_t *task, char adjustment)
{
    char priority = task->priority;
    assert((priority < TASK_PRIORITY_REALTIME));
    if ((priority < TASK_PRIORITY_REALTIME)) {
        priority = priority + adjustment;
        if (priority >= TASK_PRIORITY_REALTIME)
            priority = TASK_PRIORITY_REALTIME - 1;
        if (priority < task->static_priority)
            priority = task->static_priority;
    }
    return (uint8_t) priority;
}

static task_t *sched_queue_fetch_first(sched_unit_t *su)
{
    task_t *task;
    /* 总是选择优先级比较高的队列 */
    while (!su->priority_queue[su->dynamic_priority].length) {
        --su->dynamic_priority;
    }
    sched_queue_t *queue = &su->priority_queue[su->dynamic_priority];
    task = list_first_owner(&queue->list, task_t, list);
    --queue->length;
    --su->tasknr;
    list_del_init(&task->list);
    return task;
}

task_t *get_next_task(sched_unit_t *su)
{
    task_t *task = su->cur;
    switch (task->state) {
    case TASK_RUNNING:
        task->ticks = task->timeslice;
        task->state = TASK_READY;
    case TASK_READY:
        // Non-real-time tasks are dynamically prioritized    
        if (task->priority < TASK_PRIORITY_REALTIME && task->priority > TASK_PRIORITY_LOW) {
            task->priority--;
            if ((task->priority <= TASK_PRIORITY_LOW)) {
                task->priority = task->static_priority;
            }
        }
        sched_queue_add_tail(su, task);
    default:
        break;
    }
    task_t *next;
    next = sched_queue_fetch_first(su);
    return next;
}

static void sched_set_next_task(sched_unit_t *su, task_t *next)
{
    fpu_save(&su->cur->fpu);
    su->cur = next;
    task_activate_when_sched(su->cur);
    fpu_restore(&next->fpu);
}

void schedule()
{
    unsigned long flags;
    interrupt_save_and_disable(flags);    
    sched_unit_t *su = sched_get_cur_unit();
    task_t *next = get_next_task(su);
    task_t *cur = su->cur;
    #if DEBUG_SCHED == 1
    dbgprint("sched: switch from %d to %d\n", cur->pid, next->pid);
    #endif
    sched_set_next_task(su, next);
    thread_switch_to_next(cur, next);
    interrupt_restore_state(flags);
}

void sched_print_queue(sched_unit_t *su)
{
    if (su == NULL) {
        keprint(PRINT_ERR "[sched]: unit null!\n");
        return;
    }
    keprint(PRINT_INFO "[sched]: queue list:\n");
    sched_queue_t *queue;
    task_t *task;
    int i; 
    unsigned long flags;
    spin_lock_irqsave(&scheduler.lock, flags);
    for (i = 0; i < TASK_PRIORITY_MAX_NR; i++) {
        queue = &su->priority_queue[i];
        if (queue->length > 0) {
            keprint(PRINT_NOTICE "qeuue prio: %d\n", queue->priority);
            list_for_each_owner (task, &queue->list, list) {
                keprint(PRINT_INFO "task=%s pid=%d prio=%d ->", task->name, task->pid, task->priority);
            }
            keprint(PRINT_NOTICE "\n");
        }
    }
    spin_unlock_irqrestore(&scheduler.lock, flags);
}

void init_sched_unit(sched_unit_t *su, cpuid_t cpuid, unsigned long flags)
{
    su->cpuid = cpuid;
    su->flags = flags;
    su->cur = NULL;
    su->idle = NULL;
    spinlock_init(&su->lock);
    su->tasknr = 0;
    su->dynamic_priority = 0;
    sched_queue_t *queue;
    int i;
    for (i = 0; i < TASK_PRIORITY_MAX_NR; i++) {
        queue = &su->priority_queue[i];
        queue->priority = i;
        queue->length = 0;
        list_init(&queue->list);
        spinlock_init(&queue->lock);
    }
}

void schedule_init()
{
    scheduler.tasknr = 0;
    spinlock_init(&scheduler.lock);
    cpuid_t cpu_list[CPU_NR_MAX];
    cpu_get_attached_list(cpu_list, &scheduler.cpunr);
    int i;
    for (i = 0; i < scheduler.cpunr; i++) {
        init_sched_unit(&scheduler.sched_unit_table[i], cpu_list[i], 0);
    }
}
