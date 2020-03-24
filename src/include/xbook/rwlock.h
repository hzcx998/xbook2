#ifndef _XBOOK_RWLOCK_H
#define _XBOOK_RWLOCK_H
/* 读写锁机制：
读者优先
写者优先
读写公平
 */
#include "mutexlock.h"

/* rwlock 读写优先参数 */
enum rwlock_arg {
    RWLOCK_RD_FIRST = 0,    /* 读者优先 */
    RWLOCK_WR_FIRST,    /* 写着优先 */
    RWLOCK_RW_FAIR,     /* 读写公平 */
};

typedef struct rwlock {
    int count;                  /* 读者数 */
    mutexlock_t count_mutex;    /* 保护count更新时的互斥 */
    mutexlock_t rw_mutex;       /* 保证读者和写着互斥访问资源 */          
    mutexlock_t write_mutex;    /* 用于实现“写优先” */
    
    /* 读写锁上锁和解锁函数指针 */
    void (*read_lock)(struct rwlock *);
    void (*write_lock)(struct rwlock *);
    void (*read_unlock)(struct rwlock *);
    void (*write_unlock)(struct rwlock *);
} rwlock_t;

#define RWLOCK_INIT_RD_FIRST(lockname) \
    { .count = 0 \
    , .count_mutex = MUTEX_LOCK_INIT((lockname).count_mutex) \
    , .rw_mutex = MUTEX_LOCK_INIT((lockname).rw_mutex) \
    , .write_mutex = MUTEX_LOCK_INIT((lockname).write_mutex) \
    , .read_lock = __rwlock_read_lock_rd_first \
    , .read_unlock = __rwlock_read_unlock_rd_first \
    , .write_lock = __rwlock_write_lock_rd_first \
    , .write_unlock = __rwlock_write_unlock_rd_first \
    }
#define RWLOCK_INIT_WR_FIRST(lockname) \
    { .count = 0 \
    , .count_mutex = MUTEX_LOCK_INIT((lockname).count_mutex) \
    , .rw_mutex = MUTEX_LOCK_INIT((lockname).rw_mutex) \
    , .write_mutex = MUTEX_LOCK_INIT((lockname).write_mutex) \
    , .read_lock = __rwlock_read_lock_wr_first \
    , .read_unlock = __rwlock_read_unlock_wr_first \
    , .write_lock = __rwlock_write_lock_wr_first \
    , .write_unlock = __rwlock_write_unlock_wr_first \
    }

#define RWLOCK_INIT_RW_FAIR(lockname) \
    { .count = 0 \
    , .count_mutex = MUTEX_LOCK_INIT((lockname).count_mutex) \
    , .rw_mutex = MUTEX_LOCK_INIT((lockname).rw_mutex) \
    , .write_mutex = MUTEX_LOCK_INIT((lockname).write_mutex) \
    , .read_lock = __rwlock_read_lock_rw_fair \
    , .read_unlock = __rwlock_read_unlock_rw_fair \
    , .write_lock = __rwlock_write_lock_rw_fair \
    , .write_unlock = __rwlock_write_unlock_rw_fair \
    }

#define DEFINE_RWLOCK_RD_FIRST(lockname) \
    rwlock_t lockname = RWLOCK_INIT_RD_FIRST(lockname)
#define DEFINE_RWLOCK_WR_FIRST(lockname) \
    rwlock_t lockname = RWLOCK_INIT_WR_FIRST(lockname)
#define DEFINE_RWLOCK_RW_FAIR(lockname) \
    rwlock_t lockname = RWLOCK_INIT_RW_FAIR(lockname)

/* ----读优先---- */
static inline void __rwlock_read_lock_rd_first(rwlock_t *lock)
{
    mutex_lock(&lock->count_mutex); /* 互斥访问count变量 */
    if (lock->count == 0)   /* 当第一个读进程读取共享资源时 */
        mutex_lock(&lock->rw_mutex); /* 阻止写进程写 */
    lock->count++;  /* 读者计数加1 */
    mutex_unlock(&lock->count_mutex); /* 释放对count的互斥 */
}

static inline void __rwlock_read_unlock_rd_first(rwlock_t *lock)
{
    mutex_lock(&lock->count_mutex); /* 互斥访问count变量 */
    lock->count--;      /* 读者计数减1 */
    if (lock->count == 0)   /* 当最后一个读进程读取共享资源时 */
        mutex_unlock(&lock->rw_mutex); /* 允许写进程写 */
    mutex_unlock(&lock->count_mutex); /* 释放对count的互斥 */
}

static inline void __rwlock_write_lock_rd_first(rwlock_t *lock)
{
    mutex_lock(&lock->rw_mutex); /* 互斥访问读写资源 */
}

static inline void __rwlock_write_unlock_rd_first(rwlock_t *lock)
{
    mutex_unlock(&lock->rw_mutex); /* 互斥访问读写资源 */
}

/* ----写优先---- */
static inline void __rwlock_read_lock_wr_first(rwlock_t *lock)
{
    mutex_lock(&lock->write_mutex); /* 在无写进程请求时进入 */
    mutex_lock(&lock->count_mutex); /* 互斥访问count变量 */
    if (lock->count == 0)   /* 当第一个读进程读取共享资源时 */
        mutex_lock(&lock->rw_mutex); /* 阻止写进程写 */
    lock->count++;  /* 读者计数加1 */
    mutex_unlock(&lock->count_mutex); /* 释放对count的互斥 */
    mutex_unlock(&lock->write_mutex); /* 恢复对共享资源的访问 */
}

#define __rwlock_read_unlock_wr_first __rwlock_read_unlock_rd_first

static inline void __rwlock_write_lock_wr_first(rwlock_t *lock)
{
    mutex_lock(&lock->write_mutex); /* 在无写进程请求时进入 */
    mutex_lock(&lock->rw_mutex); /* 互斥访问读写资源 */
}

static inline void __rwlock_write_unlock_wr_first(rwlock_t *lock)
{
    mutex_unlock(&lock->rw_mutex); /* 互斥访问读写资源 */
    mutex_unlock(&lock->write_mutex); /* 恢复对共享资源的访问 */
}

/* ----读写公平---- */
#define __rwlock_read_lock_rw_fair __rwlock_read_lock_wr_first
#define __rwlock_read_unlock_rw_fair __rwlock_read_unlock_wr_first

static inline void __rwlock_write_lock_rw_fair(rwlock_t *lock)
{
    mutex_lock(&lock->write_mutex); /* 在无写进程请求时进入 */
    mutex_lock(&lock->rw_mutex); /* 互斥访问读写资源 */
    mutex_unlock(&lock->write_mutex); /* 恢复对共享资源的访问 */
}

static inline void __rwlock_write_unlock_rw_fair(rwlock_t *lock)
{
    mutex_unlock(&lock->rw_mutex); /* 互斥访问读写资源 */
}




void rwlock_init(rwlock_t *lock, enum rwlock_arg arg);

static inline void rwlock_rdlock(rwlock_t *lock)
{
    if (lock->read_lock)
        lock->read_lock(lock);
}

static inline void rwlock_wrlock(rwlock_t *lock)
{
    if (lock->write_lock)
        lock->write_lock(lock);
}

static inline void rwlock_rdunlock(rwlock_t *lock)
{
    if (lock->read_unlock)
        lock->read_unlock(lock);
}

static inline void rwlock_wrunlock(rwlock_t *lock)
{
    if (lock->write_unlock)
        lock->write_unlock(lock);
}

#endif /* _XBOOK_RWLOCK_H */
