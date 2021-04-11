#ifndef _SYS_PTHREAD_H
#define _SYS_PTHREAD_H

#include <stddef.h>
#include <stdint.h>
#include <arch/atomic.h>
#include <sys/time.h>

/* thread type */
typedef unsigned long pthread_t;

/* 进程控制 */
#define PTHREAD_PROCESS_PRIVATE     0
#define PTHREAD_PROCESS_SHARE       1


/* 线程栈参数 */
#define PTHREAD_STACKSIZE_MIN       (16 * 1024) /* 最小栈大小 */   
#define PTHREAD_STACKSIZE_DEL       PTHREAD_STACKSIZE_MIN

/* 启动状态 */
#define PTHREAD_CREATE_DETACHED     1 /* 分离状态启动 */
#define PTHREAD_CREATE_JOINABLE     0 /* 正常启动进程 */

/* 取消状态 */
#define PTHREAD_CANCEL_ENABLE       1   /* 收到信号后设置为CANCEL状态 */
#define PTHREAD_CANCEL_DISABLE      0   /* 收到信号忽略信号继续运行 */

/* 取消动作执行时机 */
#define PTHREAD_CANCEL_DEFFERED     2   /* 收到信号后继续运行至下一个取消点再退出 */
#define PTHREAD_CANCEL_ASYCHRONOUS  3   /* 立即执行取消动作（退出） */

/* pthread attr struct */
typedef struct __pthread_attr {
    int     detachstate;            /* 线程的分离状态 */
    void *  stackaddr;              /* 线程栈的位置 */
    size_t  stacksize;              /* 线程栈的大小 */
} pthread_attr_t;

/* pthread core  */
int pthread_create(
    pthread_t *thread,
    pthread_attr_t *attr,
    void * (*start_routine) (void *),
    void *arg
);
void pthread_exit(void *retval);
int pthread_join(pthread_t thread, void **thread_return);
int pthread_detach(pthread_t thread);
pthread_t pthread_self(void);
int pthread_equal(pthread_t thread1, pthread_t thread2);
int pthread_cancel(pthread_t thread);
int pthread_setcancelstate(int state, int *oldstate);
int pthread_setcanceltype(int type, int *oldtype);
void pthread_testcancel(void);

/* pthread attr */
int pthread_attr_init(pthread_attr_t *attr);
int pthread_attr_destroy(pthread_attr_t *attr);
int pthread_attr_getdetachstate(const pthread_attr_t *attr, int *detachstate);
int pthread_attr_setdetachstate(pthread_attr_t *attr, int detachstate);    
int pthread_attr_getstacksize(const pthread_attr_t *attr, size_t *stacksize);
int pthread_attr_setstacksize(pthread_attr_t *attr, size_t stacksize);
int pthread_attr_getstackaddr(const pthread_attr_t *attr, void **stackaddr);
int pthread_attr_setstackaddr(pthread_attr_t *attr, void *stackaddr);

/* spin lock: TAS lock */
typedef struct __pthread_spinlock {
    atomic_t count;                 /* 锁值: 1上锁，0，没上锁 */
    //volatile unsigned int count;        /* 锁值: 1上锁，0，没上锁 */
    int pshared;                    /* 是否在多进程之间使用 */
} pthread_spinlock_t;

/* 默认初始化为未初始化的自旋锁 */
#define PTHREAD_SPIN_LOCK_INITIALIZER \
        {ATOMIC_INIT(0), PTHREAD_PROCESS_PRIVATE}

int pthread_spin_init(pthread_spinlock_t *lock, int pshared);
int pthread_spin_destroy(pthread_spinlock_t *lock);
int pthread_spin_lock(pthread_spinlock_t *lock);
int pthread_spin_trylock(pthread_spinlock_t *lock);
int pthread_spin_unlock(pthread_spinlock_t *lock);
int pthread_spin_is_locked(pthread_spinlock_t *lock);


/* mutex lock */
#define PTHREAD_MUTEX_NORMAL            1   /* 普通互斥 */
#define PTHREAD_MUTEX_ERRORCHECK        2   /* 错误检查 */
#define PTHREAD_MUTEX_RECURSIVE         3   /* 一个线程可以多次申请此锁 */
#define PTHREAD_MUTEX_DEFAULT           PTHREAD_MUTEX_NORMAL    /* 默认锁 */

typedef struct __pthread_mutexattr {
    int pshared;        /* 斥锁范围 */
    int type;           /* 互斥锁类型 */
} pthread_mutexattr_t;

/* 默认初始化为未初始化的自旋锁 */
#define PTHREAD_MUTEX_ATTR_INITIALIZER \
        {PTHREAD_PROCESS_PRIVATE, PTHREAD_MUTEX_DEFAULT}

typedef struct __pthread_mutex {
    int lock;                       /* 锁值：1锁可以获取，0，锁不可以获取 */
    int count;                      /* 可重入时owner持有锁的次数 */
    int owner;                      /* 锁的持有者 */
    int kind;                       /* 锁的类型 */
    int mutex_queue;                    /* 对应的内核的用户等待队列 */
    pthread_spinlock_t spin;        /* 维护操作时的自旋锁 */
    pthread_mutexattr_t mattr;      /* 属性 */
} pthread_mutex_t;
/* 默认初始化为未初始化的自旋锁 */
#define PTHREAD_MUTEX_INITIALIZER \
        {.lock = 1, \
         .count = 0, \
         .owner = 0, \
         .kind = PTHREAD_MUTEX_DEFAULT, \
         .mutex_queue = -1, \
         .spin = PTHREAD_SPIN_LOCK_INITIALIZER, \
         .mattr = PTHREAD_MUTEX_ATTR_INITIALIZER}

int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr);
int pthread_mutex_destroy(pthread_mutex_t *mutex);
int pthread_mutex_lock(pthread_mutex_t *mutex);
int pthread_mutex_unlock(pthread_mutex_t *mutex);
int pthread_mutex_trylock(pthread_mutex_t *mutex);

int pthread_mutexattr_init(pthread_mutexattr_t *mattr);
int pthread_mutexattr_destroy(pthread_mutexattr_t *mattr);
int pthread_mutexattr_setpshared(pthread_mutexattr_t *mattr, int pshared);
int pthread_mutexattr_getpshared(pthread_mutexattr_t *mattr, int *pshared);
int pthread_mutexattr_settype(pthread_mutexattr_t *mattr , int type);
int pthread_mutexattr_gettype(pthread_mutexattr_t *mattr , int *type);
 

typedef struct __pthread_condattr {
    int pshared;                        /* 分析属性 */
} pthread_condattr_t;

/* 默认初始化为未初始化的自旋锁 */
#define PTHREAD_COND_ATTR_INITIALIZER \
        {PTHREAD_PROCESS_PRIVATE}

typedef struct __pthread_cond {
    pthread_spinlock_t spin;        /* 维护操作时的自旋锁 */
    int mutex_queue;                    /* 对应的内核的用户等待队列 */
    pthread_condattr_t cond_attr;      /* 属性 */
} pthread_cond_t;
/* 默认初始化为未初始化的自旋锁 */
#define PTHREAD_COND_INITIALIZER \
        {.spin = PTHREAD_SPIN_LOCK_INITIALIZER, \
         .mutex_queue = -1, \
         .cond_attr = PTHREAD_COND_ATTR_INITIALIZER}

int pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *condattr);
int pthread_cond_destroy(pthread_cond_t *cond);
int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex);
int pthread_cond_timedwait(
    pthread_cond_t *cond,
    pthread_mutex_t *mutex,
    const struct timespec *abstime
);
int pthread_cond_signal(pthread_cond_t *cond);
int pthread_cond_broadcast(pthread_cond_t *cond);

int pthread_condattr_init(pthread_condattr_t *mattr);
int pthread_condattr_destroy(pthread_condattr_t *mattr);
int pthread_condattr_getpshared(pthread_condattr_t *attr, int *pshared);
int pthread_condattr_setpshared(pthread_condattr_t *attr, int pshared);

void pthread_cleanup_push(void (*routine)(void *), void *arg);
void pthread_cleanup_pop(int execute);


#endif  /* _SYS_PTHREAD_H */