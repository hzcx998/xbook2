#include <xbook/schedule.h>
#include <xbook/task.h>
#include <assert.h>
#include <xbook/debug.h>
#include <arch/interrupt.h>
#include <arch/task.h>

// #define DEBUG_SCHED

/* 多任务调度器 */
scheduler_t scheduler;

/*
TODO：优化动态优先级：当一个优先级高的线程长期获得优运行时，可以适当调低优先级。
当一个低先级高的线程长期没获得运行时，可以适当调提高优先级。
当前采取的是高优先级执行固定时间后就降低，不太好，没体现优先级的优势。
*/

static task_t *task_priority_queue_fetch_first(sched_unit_t *su)
{
    task_t *task;
    /* 尝试调整到最低的优先级 */
    while (!su->priority_queue[su->dynamic_priority].length)
    {
        su->dynamic_priority++;
    }
    #ifdef DEBUG_SCHED
    printk("[sched]: fetch priority %d\n", su->dynamic_priority);
    #endif /* DEBUG_SCHED */
    //printk(KERN_NOTICE "highest prio=%d\n", highest_prio_queue->priority);
    priority_queue_t *queue = &su->priority_queue[su->dynamic_priority];
    
    /* get first task */
    task = list_first_owner(&queue->list, task_t, list);
    --queue->length;   /* sub a task */
    --su->tasknr;

    list_del_init(&task->list);

    return task;
}

task_t *get_next_task(sched_unit_t *su)
{
    task_t *task = su->cur;
    //printk("cur %s-%d ->", task->name, task->pid);
    /* 1.如果是时间片到了就插入到就绪队列，准备下次调度，如果是其他状态就不插入就绪队列 */
    if (task->state == TASK_RUNNING) {
        /* NOTE: 设置动态优先级 */
        task->priority++;
        if (task->priority >= MAX_PRIORITY_NR) {    // 到达最低优先级后，翻转为自己的最高优先级
            // printk(KERN_ERR "task %s prio %d static %d\n", task->name, task->priority, task->static_priority);
            task->priority = task->static_priority;
        }
        /* 时间片到了，加入就绪队列 */
        task_priority_queue_add_tail(su, task);
        // 更新信息
        task->ticks = task->timeslice;
        task->state = TASK_READY;
    /* 如果是主动让出cpu，那么就只插入就绪队列，不修改ticks */
    } else if (task->state == TASK_READY) {
        /* NOTE: 设置动态优先级 */
        task->priority++;
        if (task->priority >= MAX_PRIORITY_NR) {    // 到达最低优先级后，翻转为自己的最高优先级
            task->priority = task->static_priority;
        }
        /* 让出cpu，加入就绪队列 */
        task_priority_queue_add_tail(su, task);
    }
    /* 2.从就绪队列中获取一个任务 */
    /* 一定能够找到一个任务，因为最后的是idle任务 */
    task_t *next;
    next = task_priority_queue_fetch_first(su);
    return next;
}

static void set_next_task(sched_unit_t *su, task_t *next)
{
    su->cur = next;
    task_activate(su->cur);
}

/**
 * Schedule - 任务调度
 * 
 * 把自己放入队列末尾。
 * 从优先级最高的队列中选取一个任务来执行。
 */
void schedule()
{
    unsigned long flags;
    interrupt_save_state(flags);
    
    sched_unit_t *su = sched_get_unit();
    task_t *next = get_next_task(su);
    
    task_t *cur = su->cur;
#ifdef DEBUG_SCHED 
    printk(KERN_INFO "schedule: switch from %s-%d-%d-%d to %s-%d-%x-%d\n",
        cur->name, cur->pid, cur->timeslice, cur->priority, next->name, next->pid, next->timeslice, next->priority);
    /*thread_kstack_dump(next->kstack);
    dump_task(next);*/
#endif
    set_next_task(su, next);
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
    priority_queue_t *queue;
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
    priority_queue_t *queue;
    int i;
    for (i = 0; i < MAX_PRIORITY_NR; i++) {
        queue = &su->priority_queue[i];
        queue->priority = i;    /* 队列的优先级 */
        queue->length = 0;
        INIT_LIST_HEAD(&queue->list);
        spinlock_init(&queue->lock);
    }
}

void init_schedule()
{
    scheduler.tasknr = 0;
    spinlock_init(&scheduler.lock);
    scheduler.cpunr = CPU_NR;
    /* 读取cpu的id到cpu列表，然后传递给调度器 */
    cpuid_t cpu_list[CPU_NR];
    // TODO: get cpu id from hw
    cpu_list[0] = 0x86;

    int i;
    for (i = 0; i < scheduler.cpunr; i++) {
        init_sched_unit(&scheduler.sched_unit_table[i], cpu_list[i], 0);
    }
}
