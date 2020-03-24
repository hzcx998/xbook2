/*
同步锁：
    用于同步，让进程可以有序地执行
*/
#ifndef _XBOOK_SYNCLOCK_H
#define _XBOOK_SYNCLOCK_H

#include <xbook/semaphore.h>
#include <xbook/task.h>
#include <xbook/assert.h>

typedef struct synclock {
	task_t *holder;			// 锁的持有者
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

/**
 * synclock_init - 初始化同步锁
 * @lock: 锁对象
 */
static inline void synclock_init(synclock_t *lock)
{
    lock->holder = NULL;        // 最开始没有持有者
    lock->reapt_count = 0; // 一次都没有重复
    
    // 初始信号量为1，表示可以被使用一次
    semaphore_init(&lock->semaphore, 1);  
}

/**
 * sync_lock - 同步锁加锁
 * @lock: 锁对象
 */
static inline void sync_lock(synclock_t *lock)
{
    if (lock->holder != current_task) { // 自己没有锁才获取信号量
        /* 获取信号量，如果信号量已经被其它任务占用，那么自己就等待
        直到信号量被其它任务释放，自己才可以获取*/
        semaphore_down(&lock->semaphore);
        lock->holder = current_task; // 当自己成功获取信号量后才把锁的持有者设置成当前任务
        ASSERT(lock->reapt_count == 0); // 有新的任务第一次获取锁，说明锁还没有被任务重复获取
        lock->reapt_count = 1; // 现在被新的任务获取了，所以现在有1次获取
    } else {
        /*自己已经获取锁，在这里又来获取，所以就直接增加重复次数，而不执行获取信号量的操作*/ 
        lock->reapt_count++;
    }   
}

/**
 * sync_unlock - 同步锁解锁
 * @lock: 锁对象
 */
static inline void sync_unlock(synclock_t *lock)
{
    ASSERT(lock->holder == current_task); // 释放的时候自己必须持有锁
    if (lock->reapt_count > 1) { // 如果自己获取多次，那么只有等次数为1时才会真正释放信号量
        lock->reapt_count--; // 减少重复次数并返回
        return;
    }
    // 到这儿的话，就可以真正释放信号量了，相当于到达最外层的锁释放
    ASSERT(lock->reapt_count == 1);    
    lock->holder = NULL;    // 没有锁的持有者，锁处于未持有状态
    lock->reapt_count = 0;  // 释放后没有任务持有锁，重复次数为0
    semaphore_up(&lock->semaphore);      // 执行信号量up操作，释放信号量
}

#endif   /* _XBOOK_SYNCLOCK_H */
