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

int can_preempt;

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
    /*
    printk(KERN_INFO "> switch from %s-%d-%x-%d to %s-%d-%x-%d\n",
        cur->name, cur->pid, cur, cur->priority, next->name, next->pid, next, next->priority);
    */
    /*dump_task_kstack(next->kstack);
    dump_task(next);*/
    switch_to(cur, next);
}

#ifdef CONFIG_PREEMPT
/**
 * 抢占时机：
 * 1.创建一个新任务
 * 2.任务被唤醒，并且优先级更高
 */
void schedule_preempt(task_t *robber)
{
    task_t *cur = current_task;
    
    if (!can_preempt)
        return;

    /* 自己不能抢占自己 */
    if (cur == robber)
        return; 
#if DEBUG_LOCAL == 1
    printk(KERN_DEBUG "schedule_preempt: task %d preempt %d!\n",
        robber->pid,cur->pid);
#endif    
    /* 当前任务一顶是在运行中，当前任务被其它任务抢占
    把当前任务放到自己的优先级队列的首部，保留原有时间片
     */
    cur->state = TASK_READY;
    task_priority_queue_add_tail(cur);
    
    /* 抢占者在抢占前已经加入就绪队列，因此需要从就绪队列中删除 */
    --highest_prio_queue->length;   /* sub a task */
    robber->prio_queue = NULL;
    list_del_init(&robber->list);
    
    set_next_task(robber);

    /* 切换到该任务运行 */
    switch_to(cur, robber);
}
#endif

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
    can_preempt = 0; /* 还不能抢占 */
}
