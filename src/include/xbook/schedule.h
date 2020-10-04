
#ifndef _XBOOK_SCHEDULE_H
#define _XBOOK_SCHEDULE_H

#include "task.h"
#include <list.h>
#include <assert.h>
#include "debug.h"
#include "schedule.h"

/*任务优先级 */
enum task_priority {
    TASK_PRIO_BEST = 0,     /* 最佳优先级 */
    TASK_PRIO_RT,           /* 实时优先级 */
    TASK_PRIO_USER,         /* 用户优先级 */
    TASK_PRIO_IDLE,         /* IDLE优先级 */
    MAX_PRIORITY_NR         /* 优先级队列数量 */
};

typedef struct priority_queue {
    spinlock_t lock;        /* 自旋锁 */
    list_t list;            /* 任务链表 */
    unsigned long length;   /* 队列长度 */
    unsigned int priority;  /* 优先级 */
} priority_queue_t;

/* 调度单元，每个处理器一个调度单元 */
typedef struct {
    spinlock_t lock;        /* 自旋锁 */
    cpuid_t cpuid;          /* 调度单元的cpuid */
    uint32_t flags;         /* 调度标志 */
    uint32_t tasknr;        /* 任务数量 */
    uint32_t dynamic_priority;      /* 当前的动态优先级 */
    task_t *idle;           /* 当前的idle任务 */
    task_t *cur;            /* 当前的任务 */
    priority_queue_t priority_queue[4];  /* 优先级队列 */
} sched_unit_t;

/* 调度器 */
typedef struct _scheduler {
    spinlock_t lock;        /* 自旋锁 */
    uint32_t cpunr;         /* cpu数量 */
    uint32_t tasknr;        /* 任务的总数量 */
    sched_unit_t sched_unit_table[CPU_NR];
} scheduler_t;

extern scheduler_t scheduler;

void schedule();
void init_schedule();

/**
 * 通过cpu获取当前的调度单元
 */
static inline sched_unit_t *sched_get_unit()
{
    sched_unit_t *su = NULL;
    cpuid_t cpuid = hal_cpu_cur_get_id();
    int i;
    for (i = 0; i < scheduler.cpunr; i++) {
        su = &scheduler.sched_unit_table[i];
        if (su->cpuid == cpuid) {
            return su;
        }
    }
    if (unlikely(su == NULL))
        panic("[schdule]: get unit null!");
    return NULL;
}

static inline int is_task_in_priority_queue(sched_unit_t *su, task_t *task)
{
    priority_queue_t *queue = &su->priority_queue[su->dynamic_priority];
    return list_find(&task->list, &queue->list);
}

/**
 * task_priority_queue_add_tail - 把任务添加到特权级队列末尾
 * @task: 任务
 */
static inline void task_priority_queue_add_tail(sched_unit_t *su, task_t *task)
{
    
    priority_queue_t *queue = su->priority_queue + task->priority;
    //printk("> task %s tp:%d hp:%d", task->name,     task->priority, highest_prio_queue->priority);
    ASSERT(!list_find(&task->list, &queue->list));
    
    // 添加到就绪队列
    list_add_tail(&task->list, &queue->list);
    queue->length++;    /* 长度+1 */
    su->tasknr++;
    scheduler.tasknr++;

    /* 如果有更高的优先级，那么就把最高优先级指向它 */
    if (task->priority < su->dynamic_priority) {
        //printk("b tp:%d hp:%d", task->priority, highest_prio_queue->priority);
        su->dynamic_priority = task->priority;
    }

}

/**
 * task_priority_queue_add_head - 把任务添加到特权级队列头部
 * @task: 任务
 * 
 * be called in:
 * task_unblock, __semaphore_up
 * 
 */
static inline void task_priority_queue_add_head(sched_unit_t *su, task_t *task)
{
    priority_queue_t *queue = su->priority_queue + task->priority;
    ASSERT(!list_find(&task->list, &queue->list));
    
    // 添加到就绪队列
    list_add_tail(&task->list, &queue->list);
    queue->length++;    /* 长度+1 */
    su->tasknr++;
    scheduler.tasknr++;
    
    /* 如果有更高的优先级，那么就把最高优先级指向它 */
    if (task->priority < su->dynamic_priority) {
        //printk("b tp:%d hp:%d", task->priority, highest_prio_queue->priority);
        su->dynamic_priority = task->priority;
    }
}

void sched_print_queue(sched_unit_t *su);


#define current_task    sched_get_unit()->cur

#endif   /* _XBOOK_SCHEDULE_H */
