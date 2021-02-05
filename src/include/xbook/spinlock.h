#ifndef _XBOOK_SPINLOCK_H
#define _XBOOK_SPINLOCK_H

#include <arch/atomic.h>
#include <arch/interrupt.h>

typedef struct spinlock {
    atomic_t count; /* 锁变量是原子，为0表示空闲，1表示上锁 */
} spinlock_t;

#define SPIN_LOCK_INIT_UNLOCKED() \
    { .count = ATOMIC_INIT(0) }

#define SPIN_LOCK_INIT_LOCKED() \
    { .count = ATOMIC_INIT(1) }

#define SPIN_LOCK_INIT()  SPIN_LOCK_INIT_UNLOCKED()

#define DEFINE_SPIN_LOCK_UNLOCKED(lockname) \
    spinlock_t lockname = { .count = ATOMIC_INIT(0) }

#define DEFINE_SPIN_LOCK_LOCKED(lockname) \
    Spinlock_t lockname = { .count = ATOMIC_INIT(1) }

#define DEFINE_SPIN_LOCK(lockname) \
    DEFINE_SPIN_LOCK_UNLOCKED(lockname) 

#define spinlock_init(lock) \
    atomic_set(&(lock)->count, 0)

/* while old value is 1， it means the lock had used，wait here. */
#define spin_lock(lock) \
    do { \
        if (atomic_xchg(&(lock)->count, 1) != 1) \
            break; \
    } while(1)

#define spin_unlock(lock) \
    atomic_set(&(lock)->count, 0)

#define spin_lock_irqsave(lock, flags) \
    do { \
        interrupt_save_and_disable(flags); \
        spin_lock(lock); \
    } while (0)
    
    
#define spin_unlock_irqrestore(lock, flags) \
    do { \
        spin_unlock(lock); \
        interrupt_restore_state(flags); \
    } while (0)
    
#define spin_lock_irq(lock) \
    do { \
        interrupt_disable(); \
        spin_lock(lock); \
    } while (0)
    

#define spin_unlock_irq(lock) \
    do { \
        spin_unlock(lock); \
        interrupt_enable(); \
    } while (0)
    
/**
 * 非阻塞式获取锁
 * 如果锁已经被使用，就返回一个非0值，不会自旋等待锁释放。
 * 如果成功获得了锁，就返回0
 */
static inline int spin_try_lock(spinlock_t *lock)
{
    return atomic_xchg(&(lock)->count, 1);
}

/**
 * 如果锁已经被使用，就返回1，不然就返回0
 */
static inline int spin_is_locked(spinlock_t *lock)
{
    return atomic_get(&(lock)->count) > 0;
}

#endif   /*_BOOK_SPINLOCK_H*/
