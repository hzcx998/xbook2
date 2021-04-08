#include <pthread.h>
#include <arch/xchg.h>
#include <sys/proc.h>
#include <stdio.h>
#include <errno.h>

/* 自选一定次数后，才进行yield */
#define __SPIN_COUNT    10

int pthread_spin_init(pthread_spinlock_t *lock, int pshared)
{
    if (!lock)
        return EINVAL;
    //lock->count = 0;
    atomic_set(&lock->count, 0);
    lock->pshared = pshared;
    return 0;
}

int pthread_spin_destroy(pthread_spinlock_t *lock)
{
    if (!lock)
        return EINVAL;
    lock->pshared = 0;
    if (atomic_get(&lock->count) == 1) {
        //lock->count = 0;
        atomic_set(&lock->count, 0);
        return EBUSY;  /* 繁忙 */
    }
    //lock->count = 0;
    atomic_set(&lock->count, 0);
    return 0;
}

int pthread_spin_lock(pthread_spinlock_t *lock)
{
    if (!lock)
        return EINVAL;
    int i;
    while (1)
    {
        for (i = 0; i < __SPIN_COUNT; i++) {    /* 尝试多次后才让出cpu，以减少进程间切换。 */
            if (pthread_spin_trylock(lock) == 0)
                return 0;
        } 
        sched_yield();
    }
    return 0;
}
int pthread_spin_trylock(pthread_spinlock_t *lock)
{
    if (!lock)
        return EINVAL;
    uint32_t oldvale;
    /* 如果返回的旧值是0，那么成功获取，如果是1表示已经被占用，需要等待 */
    oldvale = atomic_xchg(&lock->count, 1);
    //oldvale = test_and_set(&lock->count, 1);
    if (oldvale != 1)   /* 没有上锁就成功 */
        return 0;
    else 
        return EBUSY;   /* EBUSY繁忙 */
}

int pthread_spin_unlock(pthread_spinlock_t *lock)
{
    if (!lock)
        return EINVAL;
    atomic_dec(&lock->count);
    //lock->count = 0;
    return 0;
}

int pthread_spin_is_locked(pthread_spinlock_t *lock)
{
    if (!lock)
        return EINVAL;
    return atomic_get(&lock->count);
}
