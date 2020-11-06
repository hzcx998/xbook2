#include <xbook/schedule.h>
#include <xbook/task.h>
#include <assert.h>
#include <xbook/debug.h>
#include <arch/interrupt.h>
#include <arch/task.h>

// #define DEBUG_SCHED

scheduler_t scheduler;
/*
TODO：优化动态优先级：当一个优先级高的线程长期获得优运行时，可以适当调低优先级。
当一个低先级高的线程长期没获得运行时，可以适当调提高优先级。
当前采取的是高优先级执行固定时间后就降低，不太好，没体现优先级的优势。
*/

static task_t *sched_queue_fetch_first(sched_unit_t *su)
{
    task_t *task;
    while (!su->priority_queue[su->dynamic_priority].length)
    {
        su->dynamic_priority++;
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
    if (task->state == TASK_RUNNING) {
        task->priority++;
        if (task->priority >= MAX_PRIORITY_NR) {
            task->priority = task->static_priority;
        }
        sched_queue_add_tail(su, task);
        task->ticks = task->timeslice;
        task->state = TASK_READY;
    } else if (task->state == TASK_READY) {
        task->priority++;
        if (task->priority >= MAX_PRIORITY_NR) {
            task->priority = task->static_priority;
        }
        sched_queue_add_tail(su, task);
    }
    task_t *next;
    next = sched_queue_fetch_first(su);
    return next;
}

static void sched_set_next_task(sched_unit_t *su, task_t *next)
{
    su->cur = next;
    task_activate_in_schedule(su->cur);
}

void schedule()
{
    unsigned long flags;
    interrupt_save_and_disable(flags);    
    sched_unit_t *su = sched_get_cur_unit();
    task_t *next = get_next_task(su);
    task_t *cur = su->cur;
#ifdef DEBUG_SCHED 
    printk(KERN_INFO "schedule: switch from %s-%d-%d-%d to %s-%d-%x-%d\n",
        cur->name, cur->pid, cur->timeslice, cur->priority, next->name, next->pid, next->timeslice, next->priority);
#endif
    sched_set_next_task(su, next);
    thread_switch_to_next(cur, next);
    interrupt_restore_state(flags);
}

void sched_print_queue(sched_unit_t *su)
{
    if (su == NULL) {
        printk(KERN_ERR "[sched]: unit null!\n");
        return;
    }
    printk(KERN_INFO "[sched]: queue list:\n");
    sched_queue_t *queue;
    task_t *task;
    int i; 
    unsigned long flags;
    spin_lock_irqsave(&scheduler.lock, flags);
    for (i = 0; i < MAX_PRIORITY_NR; i++) {
        queue = &su->priority_queue[i];
        printk(KERN_NOTICE "qeuue prio: %d\n", queue->priority);
        list_for_each_owner (task, &queue->list, list) {
            printk(KERN_INFO "task=%s pid=%d prio=%d ->", task->name, task->pid, task->priority);
        }
        printk(KERN_NOTICE "\n");
        
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
    for (i = 0; i < MAX_PRIORITY_NR; i++) {
        queue = &su->priority_queue[i];
        queue->priority = i;
        queue->length = 0;
        INIT_LIST_HEAD(&queue->list);
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
