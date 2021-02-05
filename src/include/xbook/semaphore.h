#ifndef _XBOOK_SEMAPHORE_H
#define _XBOOK_SEMAPHORE_H

#include <arch/atomic.h>
#include <xbook/waitqueue.h>
#include <xbook/memalloc.h>
#include <xbook/clock.h>
#include <xbook/mutexlock.h>

typedef struct semaphor {
	atomic_t counter;	    // 统计资源的原子变量
    mutexlock_t lock;       // 维护信号量资源的锁
	wait_queue_t waiter;	// 在此信号量上等待的进程
} semaphore_t;

#define SEMAPHORE_INIT(sema, value) \
    { .counter = ATOMIC_INIT(value) \
    , .lock = MUTEX_LOCK_INIT((sema).lock) \
    , .waiter = WAIT_QUEUE_INIT((sema).waiter) \
    }

#define DEFINE_SEMAPHORE(semaname, value) \
    semaphore_t semaname = SEMAPHORE_INIT(semaname, value)

static inline void semaphore_init(semaphore_t *sema, int value)
{
	atomic_set(&sema->counter, value);
    mutexlock_init(&sema->lock);
	wait_queue_init(&sema->waiter);
}

static inline void semaphore_destroy(semaphore_t *sema)
{
    mutex_unlock(&sema->lock);
    if (atomic_get(&sema->counter) > 0)
        wait_queue_wakeup_all(&sema->waiter);
    atomic_set(&sema->counter, 0);
}

semaphore_t *semaphore_alloc(int value);
int semaphore_free(semaphore_t *sema);

void __semaphore_down(semaphore_t *sema);

static inline void semaphore_down(semaphore_t *sema)
{
    mutex_lock(&sema->lock);
	if (atomic_get(&sema->counter) > 0) {
		atomic_dec(&sema->counter);    
        mutex_unlock(&sema->lock);
    } else {
		__semaphore_down(sema);
	}
}

static inline int semaphore_try_down(semaphore_t *sema)
{
    mutex_lock(&sema->lock);
    if (atomic_get(&sema->counter) > 0) {
		atomic_dec(&sema->counter);
    } else {
        mutex_unlock(&sema->lock);
		return -1;
	}
    mutex_unlock(&sema->lock);	
    return 0;
}

void __semaphore_up(semaphore_t *sema);

static inline void semaphore_up(semaphore_t *sema)
{
    mutex_lock(&sema->lock);
 	if (list_empty(&sema->waiter.wait_list)) {
		atomic_inc(&sema->counter);
        mutex_unlock(&sema->lock);
 	} else {
        __semaphore_up(sema);
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
