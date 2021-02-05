#ifndef _XBOOK_SYNCLOCK_H
#define _XBOOK_SYNCLOCK_H

#include <xbook/semaphore.h>
#include <xbook/task.h>
#include <xbook/schedule.h>
#include <assert.h>

typedef struct synclock {
	task_t *holder;			    // 锁的持有者
	semaphore_t semaphore;		// 用多元信号量来实现锁
	unsigned int reapt_count;	// 锁的持有者重复申请锁的次数
} synclock_t;

#define SYNC_LOCK_INIT(lockname) \
    { .holder = NULL \
    , .semaphore = SEMAPHORE_INIT((lockname).semaphore, 1) \
    , .reapt_count = 0 \
    }

#define DEFINE_SYNC_LOCK(lockname) \
    synclock_t lockname = SYNC_LOCK_INIT(lockname)

static inline void synclock_init(synclock_t *lock)
{
    lock->holder = NULL;
    lock->reapt_count = 0;
    semaphore_init(&lock->semaphore, 1);  
}

static inline void sync_lock(synclock_t *lock)
{
    if (lock->holder != task_current) {
        semaphore_down(&lock->semaphore);
        lock->holder = task_current;
        assert(lock->reapt_count == 0);
        lock->reapt_count = 1;
    } else {
        lock->reapt_count++;
    }   
}

static inline void sync_unlock(synclock_t *lock)
{
    assert(lock->holder == task_current);
    if (lock->reapt_count > 1) {
        lock->reapt_count--;
        return;
    }
    assert(lock->reapt_count == 1);    
    lock->holder = NULL;
    lock->reapt_count = 0;
    semaphore_up(&lock->semaphore);
}

#endif   /* _XBOOK_SYNCLOCK_H */
