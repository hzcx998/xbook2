#include <pthread/pthread.h>
#include <arch/atomic.h>
#include <sys/proc.h>
#include <stdio.h>

int pthread_spin_init(pthread_spinlock_t *lock, int pshared)
{
    lock->count = 0;
    lock->shared = pshared;
    return 0;
}

int pthread_spin_destroy(pthread_spinlock_t *lock)
{
    lock->shared = 0;
    if (lock->count > 0) {
        lock->count = 0;
        return -1;  /* 繁忙 */
    }
    lock->count = 0;
    return 0;
}

int pthread_spin_lock(pthread_spinlock_t *lock)
{
    /* 如果返回的旧值是0，那么成功获取，如果是1表示已经被占用，需要等待 */
    while (test_and_set(&lock->count, 1) == 1) {
        sched_yeild();
       
    }
    return 0;
}
int pthread_spin_trylock(pthread_spinlock_t *lock)
{
    uint32_t oldvale;
    oldvale = test_and_set(&lock->count, 1);
    if (!oldvale)
        return 0;
    else 
        return -1;   /* EBUSY繁忙 */
}
int pthread_spin_unlock(pthread_spinlock_t *lock)
{
    lock->count = 0;
    return 0;
}
