#ifndef _SYS_SPINLOCK_H
#define _SYS_SPINLOCK_H

#include <arch/atomic.h>

typedef struct {
	volatile int lock;
} spinlock_t;

/* 默认初始化为未初始化的自旋锁 */
#define SPIN_LOCK_INIT() \
        {.lock = 0}

int spin_lock_init(spinlock_t *lock);
int spin_lock(spinlock_t *lock);
int spin_trylock(spinlock_t *lock);
int spin_unlock(spinlock_t *lock);
int spin_is_locked(spinlock_t *lock);

#endif  /* _SYS_SPINLOCK_H */
