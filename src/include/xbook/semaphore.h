#ifndef _XBOOK_SEMAPHORE_H
#define _XBOOK_SEMAPHORE_H

#include <arch/atomic.h>
#include <xbook/waitqueue.h>
#include <xbook/memalloc.h>
#include <xbook/clock.h>

typedef struct semaphor {
	atomic_t counter;	    // 统计资源的原子变量
	wait_queue_t waiter;	// 在此信号量上等待的进程
} semaphore_t;

#define SEMAPHORE_INIT(sema, value) \
    { .counter = ATOMIC_INIT(value) \
    , .waiter = WAIT_QUEUE_INIT((sema).waiter) \
    }

#define DEFINE_SEMAPHORE(semaname, value) \
    semaphore_t semaname = SEMAPHORE_INIT(semaname, value)

static inline void semaphore_init(semaphore_t *sema, int value)
{
	atomic_set(&sema->counter, value);
	wait_queue_init(&sema->waiter);
}

static inline void semaphore_destroy(semaphore_t *sema)
{
    if (atomic_get(&sema->counter) > 0)
        wait_queue_wakeup_all(&sema->waiter);
    atomic_set(&sema->counter, 0);
}

semaphore_t *semaphore_alloc(int value);
int semaphore_free(semaphore_t *sema);

void __semaphore_down(semaphore_t *sema, unsigned long iflags);

static inline void semaphore_down(semaphore_t *sema)
{
    unsigned long flags;
    interrupt_save_and_disable(flags);
	if (atomic_get(&sema->counter) > 0) {
		atomic_dec(&sema->counter);
        interrupt_restore_state(flags);
    } else {
		__semaphore_down(sema, flags);
	}
}

static inline int semaphore_try_down(semaphore_t *sema)
{
    unsigned long flags;
    interrupt_save_and_disable(flags);
	if (atomic_get(&sema->counter) > 0) {
		atomic_dec(&sema->counter);
    } else {
		interrupt_restore_state(flags);
		return -1;
	}
    interrupt_restore_state(flags);
	return 0;
}

void __semaphore_up(semaphore_t *sema, unsigned long iflags);

static inline void semaphore_up(semaphore_t *sema)
{
    unsigned long flags;
    interrupt_save_and_disable(flags);
	if (list_empty(&sema->waiter.wait_list)) {
		atomic_inc(&sema->counter);
        interrupt_restore_state(flags);
 	} else {
        __semaphore_up(sema, flags);
	}
}

/**
 * 信号量down超时
 * 如果没有超时则返回剩余的ticks数量, 超时则返回 < 0
 */
static inline int semaphore_down_timeout(semaphore_t *sema, clock_t ticks)
{
    if (!ticks) {
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

#endif   /*_BOOK_SEMAPHORE_H */
