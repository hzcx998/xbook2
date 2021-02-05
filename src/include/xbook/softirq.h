#ifndef _XBOOK_SOFTIRQ_H
#define _XBOOK_SOFTIRQ_H

#include <arch/atomic.h>
#include <types.h>
#include <stddef.h>

#define MAX_IRQ_REDO_COUNT     10

enum {
    HIGHTASK_ASSIST_SOFTIRQ = 0,
    TIMER_SOFTIRQ,
    NET_TX_SOFTIRQ,
    NET_RX_SOFTIRQ,
    TASK_ASSIST_SOFTIRQ,
    SCHED_SOFTIRQ, 
    RCU_SOFTIRQ,    /* Preferable RCU should always be the last softirq */
    NR_SOFTIRQS
};

typedef struct softirq_action {
    void (*action)(struct softirq_action *);
} softirq_action_t;

void softirq_build(unsigned long softirq, void (*action)(softirq_action_t *));
void softirq_active(unsigned long softirq);
void softirq_init();

#define TASK_ASSIST_SCHED       0

typedef void (*task_assist_func_t)(unsigned long);

typedef struct task_assist {
    struct task_assist *next;       // 指向下一个任务协助
    unsigned long status;           // 状态
    atomic_t count;                 // 任务协助的开启与关闭，0（enable），1（disable）
    task_assist_func_t func;        // 要执行的函数
    unsigned long data;             // 回调函数的参数
} task_assist_t;

typedef struct task_assist_head {
    task_assist_t *head;
} task_assist_head_t;

/* 任务协助处理函数原型
void task_assist_handler(unsigned long);
*/

void task_assist_schedule(task_assist_t *assist);
void high_task_assist_schedule(task_assist_t *assist);

static inline void task_assist_init(task_assist_t *assist, 
        task_assist_func_t func, unsigned long data)
{
    assist->next = NULL;
    assist->status = 0;
    atomic_set(&assist->count, 0);
    assist->func = func;
    assist->data = data;
}

#define DECLEAR_TASK_ASSIST(name, func, data) \
    struct task_assist name = {NULL, 0, ATOMIC_INIT(0),func, data}
#define DECLEAR_TASK_ASSIST_DISABLED(name, func, data) \
    struct task_assist name = {NULL, 0, ATOMIC_INIT(1),func, data}

static inline void task_assist_disable(task_assist_t *assist)
{
    atomic_dec(&assist->count);
}

static inline void task_assist_enable(task_assist_t *assist)
{
    atomic_inc(&assist->count);
}

#endif  /* _XBOOK_SOFTIRQ_H */
