/* 
互斥锁：
    互斥锁（Mutex）是在原子操作API的基础上实现的信号量行为。互斥锁不能进行递归锁定或解锁，
能用于交互上下文但是不能用于中断上下文，同一时间只能有一个任务持有互斥锁，而且只有这个任务可以对互斥锁进行解锁。
当无法获取锁时，线程进入睡眠等待状态。

    互斥锁是信号量的特例。信号量的初始值表示有多少个任务可以同时访问共享资源，如果初始值为1，表示只有1个任务可以访问，
信号量变成互斥锁（Mutex）。但是互斥锁和信号量又有所区别，互斥锁的加锁和解锁必须在同一线程里对应使用，所以互斥锁只能用于线程的互斥；
信号量可以由一个线程释放，另一个线程得到，所以信号量可以用于线程的同步。

1、互斥锁和信号量比较
    a、互斥锁功能上基本与二元信号量一样，但是互斥锁占用空间比信号量小，运行效率比信号量高。所以，如果要用于线程间的互斥，优先选择互斥锁。

2、互斥锁和自旋锁比较
    a、互斥锁在无法得到资源时，内核线程会进入睡眠阻塞状态，而自旋锁处于忙等待状态。因此，如果资源被占用的时间较长，使用互斥锁较好，
        因为可让CPU调度去做其它进程的工作。

    b、如果被保护资源需要睡眠的话，那么只能使用互斥锁或者信号量，不能使用自旋锁。而互斥锁的效率又比信号量高，所以这时候最佳选择是互斥锁。

    c、中断里面不能使用互斥锁，因为互斥锁在获取不到锁的情况下会进入睡眠，而中断是不能睡眠的。
 */

#ifndef _XBOOK_MUTEX_LOCK_H
#define _XBOOK_MUTEX_LOCK_H

#include "spinlock.h"
#include "list.h"

/* 互斥锁结构 */
typedef struct mutex_lock {
    atomic_t count;             /* 为0表示未上锁，1表示上锁 */
    list_t wait_list;     /* 等待队列 */
    int waiters;    /* 等待者数量 */
} mutexlock_t;

/* 初始化互斥锁 */
#define MUTEX_LOCK_INIT(lockname) \
        { .count = ATOMIC_INIT(0) \
        , .wait_list = LIST_HEAD_INIT((lockname).wait_list) \
        , .waiters = 0 \
        }

/* 定义一个互斥锁 */
#define DEFINE_MUTEX_LOCK(lockname) \
        mutexlock_t lockname = MUTEX_LOCK_INIT(lockname)

/**
 * mutexlock_init - 初始化互斥锁
 * @mutex: 锁对象
 */
static inline void mutexlock_init(mutexlock_t *mutex)
{
    atomic_set(&mutex->count, 0);   /* 0 means unused */
    INIT_LIST_HEAD(&mutex->wait_list);
    mutex->waiters = 0;
}
void mutex_unlock(mutexlock_t *mutex);
void mutex_lock(mutexlock_t *mutex);
/**
 * mutex_try_lock - 尝试自旋锁加锁
 * @lock: 锁对象
 * 
 * 非阻塞式获取锁
 * 如果锁已经被使用，就返回一个非0值，不会自旋等待锁释放。
 * 如果成功获得了锁，就返回0
 */
static inline int mutex_try_lock(mutexlock_t *lock)
{
    return atomic_xchg(&(lock)->count, 1);
}

/**
 * mutex_is_locked - 检测锁是否被占用
 * @lock: 锁对象
 * 
 * 如果锁已经被使用，就返回1
 * 不然就返回0
 */
static inline int mutex_is_locked(mutexlock_t *lock)
{
    return atomic_get(&(lock)->count) > 0;
}

void dump_mutex(mutexlock_t *lock);

#endif   /* _XBOOK_MUTEX_LOCK_H */
