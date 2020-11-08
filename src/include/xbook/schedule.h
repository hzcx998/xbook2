
#ifndef _XBOOK_SCHEDULE_H
#define _XBOOK_SCHEDULE_H

#include "task.h"
#include <xbook/list.h>
#include <assert.h>
#include "debug.h"
#include "schedule.h"

/* 
优先级位于0-31
实时任务优先级不会动态变化。固定在16-31
较低的任务有限级可以在1-15之间变化。
 */
enum sched_priority_level {
    TASK_PRIO_LEVEL_UNKNOWN = 0,
    TASK_PRIO_LEVEL_LOW,
    TASK_PRIO_LEVEL_BELOW_NORMAL,
    TASK_PRIO_LEVEL_NORMAL,
    TASK_PRIO_LEVEL_ABOVE_NORMAL,
    TASK_PRIO_LEVEL_HIGH,
    TASK_PRIO_LEVEL_REALTIME,
    TASK_PRIO_LEVEL_MAX
};

#define TASK_PRIORITY_SYS       0
#define TASK_PRIORITY_LOW       1
#define TASK_PRIORITY_REALTIME  16
#define TASK_PRIORITY_MAX       31
#define TASK_PRIORITY_MAX_NR    (TASK_PRIORITY_MAX + 1)
#define TASK_PRIORITY_TURN_DISTANCE   7

typedef struct {
    spinlock_t lock;
    list_t list;
    unsigned long length;   /* 队列任务长度 */
    unsigned int priority;  /* 队列优先级 */
} sched_queue_t;

typedef struct {
    spinlock_t lock;
    cpuid_t cpuid;          /* 调度单元的cpuid */
    uint32_t flags;
    uint32_t tasknr;
    uint32_t dynamic_priority;      /* 当前调度单元的动态优先级 */
    task_t *idle;           /* 当前调度单元的idle任务 */
    task_t *cur;            /* 当前调度单元的执行中的任务 */
    sched_queue_t priority_queue[TASK_PRIORITY_MAX_NR];  /* 优先级队列 */
} sched_unit_t;

typedef struct _scheduler {
    spinlock_t lock;
    uint32_t cpunr;         /* cpu数量 */
    uint32_t tasknr;        /* 任务的总数量 */
    sched_unit_t sched_unit_table[CPU_NR_MAX];
} scheduler_t;

extern scheduler_t scheduler;

void schedule();
void schedule_init();

uint8_t sched_calc_base_priority(uint32_t level);
uint8_t sched_calc_new_priority(task_t *task, char adjustment);

static inline sched_unit_t *sched_get_cur_unit()
{
    sched_unit_t *su = NULL;
    cpuid_t cpuid = cpu_get_my_id();
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

static inline int sched_queue_has_task(sched_unit_t *su, task_t *task)
{
    sched_queue_t *queue = &su->priority_queue[su->dynamic_priority];
    return list_find(&task->list, &queue->list);
}

static inline void sched_queue_add_tail(sched_unit_t *su, task_t *task)
{    
    sched_queue_t *queue = su->priority_queue + task->priority;
    ASSERT(!list_find(&task->list, &queue->list));
    list_add_tail(&task->list, &queue->list);
    queue->length++;
    su->tasknr++;
    scheduler.tasknr++;
    if (task->priority > su->dynamic_priority) {
        su->dynamic_priority = task->priority;
    }
}

static inline void sched_queue_add_head(sched_unit_t *su, task_t *task)
{
    sched_queue_t *queue = su->priority_queue + task->priority;
    ASSERT(!list_find(&task->list, &queue->list));
    list_add_tail(&task->list, &queue->list);
    queue->length++;
    su->tasknr++;
    scheduler.tasknr++;    
    if (task->priority > su->dynamic_priority) {
        su->dynamic_priority = task->priority;
    }
}

void sched_print_queue(sched_unit_t *su);

#define task_current    sched_get_cur_unit()->cur

#endif   /* _XBOOK_SCHEDULE_H */
