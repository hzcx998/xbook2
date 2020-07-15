#ifndef _XBOOK_SOFTIRQ_H
#define _XBOOK_SOFTIRQ_H

#include <arch/atomic.h>
#include <types.h>
#include <stddef.h>

/* 软中断 */

/* 最大的软中断数量 */
#define MAX_NR_SOFTIRQS     32

#define MAX_REDO_IRQ     10

/* 定义软中断类型 */
enum
{
    HIGHTASK_ASSIST_SOFTIRQ = 0,
    TIMER_SOFTIRQ,
    NET_TX_SOFTIRQ,
    NET_RX_SOFTIRQ,
    TASK_ASSIST_SOFTIRQ,
    SCHED_SOFTIRQ, 
    RCU_SOFTIRQ,    /* Preferable RCU should always be the last softirq */
    NR_SOFTIRQS
};

/* 软中断行为 */
typedef struct softirq_action {
    void (*action)(struct softirq_action *);
} softirq_action_t;

/* 软中断处理函数原型
void softirq_handler(struct softirq_action *action);
*/

void build_softirq(unsigned long softirq, void (*action)(softirq_action_t *));
void active_softirq(unsigned long softirq);

void init_softirq();

/* task_assist, 任务协助 */

#define TASK_ASSIST_SCHED       0

typedef struct task_assist {
    struct task_assist *next;        // 指向下一个任务协助
    unsigned long status;            // 状态
    atomic_t count;                 // 任务协助的开启与关闭，0（enable），1（disable）
    void (*func)(unsigned long);     // 要执行的函数
    unsigned long data;              // 回调函数的参数
} task_assist_t;

typedef struct task_assist_head {
    task_assist_t *head;
} task_assist_head_t;

/* 任务协助处理函数原型
void task_assist_handler(unsigned long);
*/

void task_assist_schedule(task_assist_t *assist);
void high_task_assist_schedule(task_assist_t *assist);
/**
 * task_assist_init - 初始化一个任务协助
 * @assist: 任务协助的地址
 * @func: 要处理的函数
 * @data: 传入的参数
 */
static inline void task_assist_init(task_assist_t *assist, 
        void (*func)(unsigned long), unsigned long data)
{
    assist->next = NULL;
    assist->status = 0;
    atomic_set(&assist->count, 0);
    assist->func = func;
    assist->data = data;
}

/* 初始化一个任务协助，使能的，enable */
#define DECLEAR_TASK_ASSIST(name, func, data) \
    struct task_assist name = {NULL, 0, ATOMIC_INIT(0),func, data}


/* 初始化一个任务协助，不使能的，disable */
#define DECLEAR_TASK_ASSIST_DISABLED(name, func, data) \
    struct task_assist name = {NULL, 0, ATOMIC_INIT(1),func, data}

/**
 * task_assistDisable - 不让任务协助生效
 * @assist: 任务协助
 */
static inline void task_assist_disable(task_assist_t *assist)
{
    /* 减小次数，使其disable */
    atomic_dec(&assist->count);
}

/**
 * task_assistEnable - 让任务协助生效
 * @assist: 任务协助
 */
static inline void task_assist_enable(task_assist_t *assist)
{
    /* 增加次数，使其enable */
    atomic_inc(&assist->count);
}

#endif  /* _XBOOK_SOFTIRQ_H */
