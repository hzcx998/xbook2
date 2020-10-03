#ifndef _XBOOK_SEMAPHORE_H
#define _XBOOK_SEMAPHORE_H

#include <arch/atomic.h>
#include <xbook/waitqueue.h>
#include <xbook/schedule.h>
#include <xbook/kmalloc.h>
#include <xbook/clock.h>

typedef struct semaphor {
	atomic_t counter;			// 统计资源的原子变量
	wait_queue_t waiter;	// 在此信号量上等待的进程
} semaphore_t;

#define SEMAPHORE_INIT(sema, value) \
    { .counter = ATOMIC_INIT(value) \
    , .waiter = WAIT_QUEUE_INIT((sema).waiter) \
    }

#define DEFINE_SEMAPHORE(semaname, value) \
    semaphore_t semaname = SEMAPHORE_INIT(semaname, value)

/**
 * semaphore_Init - 信号量初始化
 * @sema: 信号量
 * @value: 初始值
 */
static inline void semaphore_init(semaphore_t *sema, int value)
{
	/* 设置成初始值 */
	atomic_set(&sema->counter, value);
	/* 初始化等待队列 */
	wait_queue_init(&sema->waiter);
}

/**
 * semaphore_Init - 信号量初始化
 * @sema: 信号量
 * @value: 初始值
 */
static inline void semaphore_destroy(semaphore_t *sema)
{
    if (atomic_get(&sema->counter) > 0)
        wait_queue_wakeup_all(&sema->waiter);
    atomic_set(&sema->counter, 0);
}

/**
 * __semaphore_down - 执行具体的down操作
 * @sema: 信号量
 */
static inline void __semaphore_down(semaphore_t *sema)
{
	/* 把自己添加到信号量的等待队列中，等待被唤醒 */
	list_add_tail(&current_task->list, &sema->waiter.wait_list);
	task_block(TASK_BLOCKED);
}

/**
 * semaphore_down - 信号量down
 * @sema: 信号量
 */
static inline void semaphore_down(semaphore_t *sema)
{
    
    unsigned long flags;
    save_intr(flags);

	/* 如果计数器大于0，就说明资源没有被占用 */
	if (atomic_get(&sema->counter) > 0) {
		/* 计数器减1，说明现在信号量被获取一次了 */
		atomic_dec(&sema->counter);
    } else {
        /* 如果信号量为0，说明不能被获取，那么就把当前进程阻塞 */
		__semaphore_down(sema);
	}
    restore_intr(flags);
}

/**
 * semaphore_try_down - 尝试执行信号量down
 * @sema: 信号量
 */
static inline int semaphore_try_down(semaphore_t *sema)
{
    
    unsigned long flags;
    save_intr(flags);

	/* 如果计数器大于0，就说明资源没有被占用 */
	if (atomic_get(&sema->counter) > 0) {
		/* 计数器减1，说明现在信号量被获取一次了 */
		atomic_dec(&sema->counter);
    } else {	
		/* 不能获取的话，就不被阻塞，直接返回 */
		restore_intr(flags);
		return -1;
	}
    restore_intr(flags);
	return 0;
}

/**
 * __semaphore_down - 执行具体的up操作
 * @sema: 信号量
 */
static inline void __semaphore_up(semaphore_t *sema)
{
	/* 从等待队列获取一个等待者 */
	task_t *waiter = list_first_owner(&sema->waiter.wait_list, task_t, list);
	
	/* 让等待者的链表从信号量的链表中脱去 */
	list_del(&waiter->list);
    
	/* 设置任务为就绪状态 */
	waiter->state = TASK_READY;
    task_priority_queue_add_head(sched_get_unit(), waiter);
}

/**
 * semaphore_up - 信号量up
 * @sema: 信号量
 */
static inline void semaphore_up(semaphore_t *sema)
{
    unsigned long flags;
    save_intr(flags);
	/* 如果等待队列为空，说明没有等待的任务，就只释放信号量 */
	if (list_empty(&sema->waiter.wait_list)) {
		/* 使信号量递增 */
		atomic_inc(&sema->counter);
 	} else {
 		/* 有等待任务，就唤醒一个 */
		__semaphore_up(sema);
	}
    restore_intr(flags);
}

/**
 * semaphore_down_timeout - 信号量down超时
 * @sema: 信号量
 * @ticks: 超时的ticks数量
 * 
 * 如果没有超时则返回剩余的ticks数量, 超时则返回 < 0
 */
static inline int semaphore_down_timeout(semaphore_t *sema, clock_t ticks)
{
    if (!ticks) {   /* 一直阻塞等待 */
        semaphore_down(sema);
        return 0;
    }
    clock_t start = sys_get_ticks();
    clock_t end = start;
    clock_t t = ticks;
    while (t > 0) {
        end = sys_get_ticks();;
        if (end > start) {
            t -= (end - start);
            start = end;
        }
        if (!semaphore_try_down(sema))
            return 0;
    }
    return -1;
}

#endif   /*_BOOK_semaphore__H*/
