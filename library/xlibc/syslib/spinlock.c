#include <arch/xchg.h>
#include <sys/proc.h>
#include <sys/spinlock.h>
#include <stdio.h>
#include <errno.h>
#include <stdint.h>

/* 自选一定次数后，才进行yeild */
#define __SPIN_COUNT    10

int spin_lock_init(spinlock_t *lock)
{
    if (!lock)
        return EINVAL;
    lock->lock = 0;
    return 0;
}

int spin_lock(spinlock_t *lock)
{
    if (!lock)
        return EINVAL;
    int i;
    while (1) {
        for (i = 0; i < __SPIN_COUNT; i++) {    /* 尝试多次后才让出cpu，以减少进程间切换。 */
            if (spin_trylock(lock) == 0)
                return 0;
        } 
        sched_yeild();
    }
    return 0;
}

int spin_trylock(spinlock_t *lock)
{
    if (!lock)
        return EINVAL;
    uint32_t oldvale;
    /* 如果返回的旧值是0，那么成功获取，如果是1表示已经被占用，需要等待 */
    oldvale = test_and_set(&lock->lock, 1);
    if (oldvale != 1)   /* 没有上锁就成功 */
        return 0;
    else 
        return EBUSY;   /* EBUSY繁忙 */
}

int spin_unlock(spinlock_t *lock)
{
    if (!lock)
        return EINVAL;
    lock->lock--;
    return 0;
}

int spin_is_locked(spinlock_t *lock)
{
    if (!lock)
        return EINVAL;
    return lock->lock;
}
