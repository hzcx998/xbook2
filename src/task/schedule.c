#include <xbook/schedule.h>
#include <xbook/task.h>
#include <xbook/assert.h>
#include <xbook/debug.h>
#include <arch/interrupt.h>
#include <arch/task.h>

extern task_t *task_idle;

/* 优先级队列 */
priority_queue_t priority_queue[MAX_PRIORITY_NR];

/* 最高级的队列 */
priority_queue_t *highest_prio_queue;

task_t *get_next_task(task_t *task)
{
    /* 1.插入到就绪队列 */
    if (task->state == TASK_RUNNING) {
        //printk("%s out", task->name);

        //if (task->pid == 0)
            //printk("schedule idle\n");
        /* 时间片到了，加入就绪队列 */
        task_priority_queue_add_tail(task);
        // 更新信息
        task->ticks = task->timeslice;
        task->state = TASK_READY;
    } else {
        /* 如果是需要某些事件后才能继续运行，不用加入队列，当前线程不在就绪队列中。*/            
        //printk("$%d ", task->priority);
        //printk("%s without ready!\n", task->name);
    }

    /* 队列为空，那么就尝试唤醒idle */
    if (is_all_priority_queue_empty()) {
        //printk("wake up idle.\n");
        // 唤醒mian(idle)
        task_unblock(task_idle);
    }
    
    /* 2.从就绪队列中获取一个任务 */
    /* 一定能够找到一个任务，因为最后的是idle任务 */
    task_t *next;

    /* 当最高优先级的长度为0，就降低最高优先级到长度不为0的优先级，直到到达最低的优先级 */
    ADJUST_HIGHEST_PRIO(highest_prio_queue);
    //printk("#%d ", highest_prio_queue->priority);
    /* get first task */
    next = list_first_owner(&highest_prio_queue->list, task_t, list);
    --highest_prio_queue->length;   /* sub a task */
    next->prio_queue = NULL;
    list_del_init(&next->list);

    /* 当最高优先级的长度为0，就降低最高优先级到长度不为0的优先级 */
    ADJUST_HIGHEST_PRIO(highest_prio_queue);
    //printk("#%d ", highest_prio_queue->priority);
    //printk("get %s", next->name);
    //printk("c tp:%d hp:%d", next->priority, highest_prio_queue->priority);
    
    //if (next->pid == 0)
        //printk("select idle\n");
        
    return next;
}    


/**
 * Schedule - 任务调度
 * 
 * 把自己放入队列末尾。
 * 从优先级最高的队列中选取一个任务来执行。
 */
void schedule()
{
    /* close intr */
    unsigned long flags;
    save_intr(flags);
    task_t *task = current_task;
    
    task_t *next = get_next_task(task);

    restore_intr(flags);
    
    /* 3.激活任务的内存环境 */
    task_activate(next);
    /* 4.切换到该任务运行 */
    switch_to(task, next);
}

#ifdef CONFIG_PREEMPT
/**
 * 抢占时机：
 * 1.创建一个新任务
 * 2.任务被唤醒，并且优先级更高
 */
void schedule_preempt(task_t *robber)
{
    /* close intr */
    unsigned long flags;
    save_intr(flags);
    
    task_t *cur = current_task;
    
    /* 自己不能抢占自己 */
    if (cur == robber)
        return;

    /* 当前任务一顶是在运行中，当前任务被其它任务抢占
    把当前任务放到自己的优先级队列的首部，保留原有时间片
     */
    cur->state = TASK_READY;
    task_priority_queue_add_tail(cur);
    
    /* 最高优先级指针已经在抢占前指向了抢占者的优先级，这里就不用调整了 */

    /* 抢占者需要从就绪队列中删除 */
    --highest_prio_queue->length;   /* sub a task */
    robber->prio_queue = NULL;
    list_del_init(&robber->list);

    /* 当最高优先级的长度为0，就降低最高优先级到长度不为0的优先级 */
    ADJUST_HIGHEST_PRIO(highest_prio_queue);
    
    restore_intr(flags);
    
    /* 3.激活任务的内存环境 */
    task_activate(robber);
    
    /* 4.切换到该任务运行 */
    switch_to(cur, robber);
}
#endif

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
