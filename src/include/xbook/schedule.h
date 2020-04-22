
#ifndef _XBOOK_SCHEDULE_H
#define _XBOOK_SCHEDULE_H

#include "task.h"
#include "list.h"
#include "assert.h"
#include "debug.h"

/*任务优先级 */
enum task_priority {
    TASK_PRIO_BEST = 0,     /* 最佳优先级 */
    TASK_PRIO_RT,           /* 实时优先级 */
    TASK_PRIO_USER,         /* 用户优先级 */
    TASK_PRIO_IDLE,         /* IDLE优先级 */
};

extern priority_queue_t *highest_prio_queue;
extern priority_queue_t priority_queue[];

/* 优先级队列数量 */
#define MAX_PRIORITY_NR  4

#define ADJUST_HIGHEST_PRIO(highest) \
    while (!(highest->length) && (highest->priority < MAX_PRIORITY_NR - 1)) highest++

#define is_task_in_priority_queue(task) (task->prio_queue == NULL ? 0 : 1) 

void schedule();
void init_schedule();
void launch_task();

/**
 * is_all_priority_queue_empty - 判断优先级队列是否为空
 */
static inline int is_all_priority_queue_empty()
{
    priority_queue_t *queue = highest_prio_queue;
    for (; queue < priority_queue + MAX_PRIORITY_NR; queue++) {
        if (queue->length > 0) /* 如果优先级队列长度大于0，说明不为空 */
            return 0;
    }
    return 1;
}

/**
 * task_priority_queue_add_tail - 把任务添加到特权级队列末尾
 * @task: 任务
 *  
 */
static inline void task_priority_queue_add_tail(task_t *task)
{
    task->prio_queue = &priority_queue[task->priority];
    //printk("> task %s tp:%d hp:%d", task->name,     task->priority, highest_prio_queue->priority);
    ASSERT(!list_find(&task->list, &task->prio_queue->list));
    
    // 添加到就绪队列
    list_add_tail(&task->list, &task->prio_queue->list);
    task->prio_queue->length++;    /* 长度+1 */

    /* 如果有更高的优先级，那么就把最高优先级指向它 */
    if (task->priority < highest_prio_queue->priority) {
        //printk("b tp:%d hp:%d", task->priority, highest_prio_queue->priority);

        highest_prio_queue = task->prio_queue;
    }
}

/**
 * task_priority_queue_add_head - 把任务添加到特权级队列头部
 * @task: 任务
 * 
 */
static inline void task_priority_queue_add_head(task_t *task)
{
    task->prio_queue = &priority_queue[task->priority];
    ASSERT(!list_find(&task->list, &task->prio_queue->list));
    
    // 添加到就绪队列
    list_add(&task->list, &task->prio_queue->list);
    task->prio_queue->length++;    /* 长度+1 */

    /* 如果有更高的优先级，那么就把最高优先级指向它 */
    if (task->priority < highest_prio_queue->priority) {
        highest_prio_queue = task->prio_queue;

    }  
}

task_t *task_priority_queue_fetch_first();

void print_priority_queue(int prio);

#endif   /* _XBOOK_SCHEDULE_H */
