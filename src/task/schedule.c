#include <xbook/schedule.h>
#include <xbook/task.h>
#include <arch/interrupt.h>

extern task_t *task_idle;
extern list_t task_priority_queue[];

/** 
 * SwitchTo - 任务切换的核心
 * @prev: 当前任务
 * @next: 要切换过去的任务
 * 
 * 切换任务时保存当前环境，再选择新任务的环境去执行
 */
extern void switch_to(task_t *prev, task_t *next);


/**
 * schedule_in_intr - 任务调度
 * 
 * 当需要从一个任务调度到另一个任务时，使用这个函数来执行操作
 * 
 * 把自己放入队列末尾。
 * 从优先级最高的队列中选取一个任务来执行。
 * 
 * 在时钟中断产生时的调度
 */
void schedule_in_intr()
{
    task_t *task = current_task;
    /* 1.插入到就绪队列 */
    if (task->state == TASK_RUNNING) {
        /* 时间片到了，加入就绪队列 */
        task_priority_queue_add_tail(task);
        // 更新信息
        task->ticks = task->timeslice;
        task->state = TASK_READY;
    } else {
        /* 如果是需要某些事件后才能继续运行，不用加入队列，当前线程不在就绪队列中。*/            
    }

    /* 尝试唤醒idle任务 */
    // 队列为空，那么就尝试唤醒idle
    if (is_all_priority_queue_empty()) {
        // 唤醒mian(idle)
        task_unblock(task_idle);
    }
    
    /* 2.从就绪队列中获取一个任务 */
    /* 一定能够找到一个任务，因为最后的是idle任务 */
    task_t *next;
    int i;
    for (i = 0; i < MAX_PRIORITY_NR; i++) {
        /* 如果有任务，才获取 */
        if (!list_empty(&task_priority_queue[i])) {
            next = list_first_owner(&task_priority_queue[i], task_t, list);
            list_del(&next->list);
            break;
        }
    }

    /* 3.激活任务的内存环境 */
    task_activate(next);

    /* 4.切换到该任务运行 */
    switch_to(task, next);
}


/**
 * Schedule - 任务调度
 * 
 * 当需要从一个任务调度到另一个任务时，使用这个函数来执行操作
 * 
 * 把自己放入队列末尾。
 * 从优先级最高的队列中选取一个任务来执行。
 * 
 * 其它情况下的调度
 */
void schedule()
{
    unsigned long flags;
    save_intr(flags);
    task_t *task = current_task;
    /* 1.插入到就绪队列 */
    if (task->state == TASK_RUNNING) {
        /* 时间片到了，加入就绪队列 */
        task_priority_queue_add_tail(task);
        // 更新信息
        task->ticks = task->timeslice;
        task->state = TASK_READY;
    } else {
        /* 如果是需要某些事件后才能继续运行，不用加入队列，当前线程不在就绪队列中。*/            
    }

    /* 尝试唤醒idle任务 */
    // 队列为空，那么就尝试唤醒idle
    if (is_all_priority_queue_empty()) {
        // 唤醒mian(idle)
        task_unblock(task_idle);
    }
    
    /* 2.从就绪队列中获取一个任务 */
    /* 一定能够找到一个任务，因为最后的是idle任务 */
    task_t *next;
    int i;
    for (i = 0; i < MAX_PRIORITY_NR; i++) {
        /* 如果有任务，才获取 */
        if (!list_empty(&task_priority_queue[i])) {
            next = list_first_owner(&task_priority_queue[i], task_t, list);
            list_del(&next->list);
            break;
        }
    }
    restore_intr(flags);

    /* 3.激活任务的内存环境 */
    task_activate(next);

    /* 4.切换到该任务运行 */
    switch_to(task, next);
}
