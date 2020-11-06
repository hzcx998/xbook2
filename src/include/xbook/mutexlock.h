#ifndef _XBOOK_MUTEX_LOCK_H
#define _XBOOK_MUTEX_LOCK_H

#include "spinlock.h"
#include <xbook/list.h>

typedef struct mutex_lock {
    spinlock_t wait_lock;
    list_t wait_list;
    int waiters;
} mutexlock_t;

#define MUTEX_LOCK_INIT(lockname) \
        { .wait_lock = SPIN_LOCK_INIT_UNLOCKED() \
        , .wait_list = LIST_HEAD_INIT((lockname).wait_list) \
        , .waiters = 0 \
        }

#define DEFINE_MUTEX_LOCK(lockname) \
        mutexlock_t lockname = MUTEX_LOCK_INIT(lockname)

static inline void mutexlock_init(mutexlock_t *mutex)
{
    spinlock_init(&mutex->wait_lock);
    INIT_LIST_HEAD(&mutex->wait_list);
    mutex->waiters = 0;
}
void mutex_unlock(mutexlock_t *mutex);
void mutex_lock(mutexlock_t *mutex);

static inline int mutex_try_lock(mutexlock_t *lock)
{
    return spin_try_lock(&lock->wait_lock);
}

static inline int mutex_is_locked(mutexlock_t *lock)
{
    return spin_is_locked(&lock->wait_lock);
}

#endif   /* _XBOOK_MUTEX_LOCK_H */
