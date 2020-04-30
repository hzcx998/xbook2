#ifndef _SYS_PTHREAD_H
#define _SYS_PTHREAD_H

#include <stddef.h>
#include <stdint.h>

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
int pthread_attr_getdetachstate(const pthread_attr_t *attr, int *detachstate);
int pthread_attr_setdetachstate(pthread_attr_t *attr, int detachstate);    
int pthread_attr_getstacksize(const pthread_attr_t *attr, size_t *stacksize);
int pthread_attr_setstacksize(pthread_attr_t *attr, size_t stacksize);
int pthread_attr_getstackaddr(const pthread_attr_t *attr, void **stackaddr);
int pthread_attr_setstackaddr(pthread_attr_t *attr, void *stackaddr);

/* spin lock: TAS lock */
typedef struct __pthread_spinlock {
    volatile uint32_t count;        /* 锁值: 1上锁，0，没上锁 */
    int shared;                    /* 是否在多进程之间使用 */
} pthread_spinlock_t;

/* 默认初始化为未初始化的自旋锁 */
#define PTHREAD_SPIN_LOCK_INITIALIZER \
        {0, PTHREAD_PROCESS_PRIVATE}

int pthread_spin_init(pthread_spinlock_t *lock, int pshared);
int pthread_spin_destroy(pthread_spinlock_t *lock);
int pthread_spin_lock(pthread_spinlock_t *lock);
int pthread_spin_trylock(pthread_spinlock_t *lock);
int pthread_spin_unlock(pthread_spinlock_t *lock);

#endif  /* _SYS_PTHREAD_H */