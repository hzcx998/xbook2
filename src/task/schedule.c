#include <xbook/schedule.h>
#include <xbook/task.h>
#include <xbook/assert.h>
#include <xbook/debug.h>
#include <arch/interrupt.h>
#include <arch/task.h>

#define DEBUG_LOCAL 0

extern task_t *task_idle;

/* 优先级队列 */
priority_queue_t priority_queue[MAX_PRIORITY_NR];

/* 最高等级的队列 */
priority_queue_t *highest_prio_queue;

task_t *task_priority_queue_fetch_first()
{
    task_t *task;
    //printk(KERN_NOTICE "highest prio=%d\n", highest_prio_queue->priority);
    
    //highest_prio_queue = &priority_queue[0];
    /* 从最高优先级开始寻找一个最近的有任务的低优先级 */
    ADJUST_HIGHEST_PRIO(highest_prio_queue);
    /* get first task */
    task = list_first_owner(&highest_prio_queue->list, task_t, list);
    --highest_prio_queue->length;   /* sub a task */
    task->prio_queue = NULL;
    list_del_init(&task->list);

    return task;
}

task_t *get_next_task(task_t *task)
{
    //printk("cur %s-%d ->", task->name, task->pid);
    /* 1.如果是时间片到了就插入到就绪队列，准备下次调度，如果是其他状态就不插入就绪队列 */
    if (task->state == TASK_RUNNING) {
        /* 时间片到了，加入就绪队列 */
        task_priority_queue_add_tail(task);
        // 更新信息
        task->ticks = task->timeslice;
        task->state = TASK_READY;
    /* 如果是主动让出cpu，那么就只插入就绪队列，不修改ticks */
    } else if (task->state == TASK_READY) {
        /* 让出cpu，加入就绪队列 */
        task_priority_queue_add_tail(task);
    }
    /* 2.从就绪队列中获取一个任务 */
    /* 一定能够找到一个任务，因为最后的是idle任务 */
    task_t *next;
    next = task_priority_queue_fetch_first();
    return next;
}

void set_next_task(task_t *next)
{
    task_current = next;
    task_activate(task_current);
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
    save_intr(flags);
    task_t *cur = current_task;
    task_t *next = get_next_task(cur);
    
    set_next_task(next);
    restore_intr(flags);
#if DEBUG_LOCAL == 1 
    printk(KERN_INFO "schedule: switch from %s-%d-%x-%d to %s-%d-%x-%d\n",
        cur->name, cur->pid, cur, cur->priority, next->name, next->pid, next, next->priority);
#endif
    /*dump_task_kstack(next->kstack);
    dump_task(next);*/
    switch_to(cur, next);
}

void print_priority_queue(int prio)
{
    if (prio < 0 || prio >= MAX_PRIORITY_NR) {
        return;
    }
    printk("print_priority_queue: list\n");
    priority_queue_t *queue = &priority_queue[prio];
    task_t *task;
    list_for_each_owner (task, &queue->list, list) {
        printk("task=%s pid=%d prio=%d vmm=%x\n", task->name, task->pid, task->priority, task->vmm);
    }
}

void init_schedule()
{
    /* 初始化特权队列 */
    int i;
    for (i = 0; i < MAX_PRIORITY_NR; i++) {
        INIT_LIST_HEAD(&priority_queue[i].list);    
        priority_queue[i].length = 0; 
        priority_queue[i].priority = i;
    }
    /* 指向最高级的队列 */
    highest_prio_queue = &priority_queue[0];
}
