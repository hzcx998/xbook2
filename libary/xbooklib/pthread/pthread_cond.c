#include <pthread/pthread.h>
#include <sys/waitque.h>
#include <sys/proc.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

static pthread_condattr_t __pthread_cond_default_attr = PTHREAD_COND_ATTR_INITIALIZER;

/**
 * pthread_cond_init - 初始化条件变量
 * 
 * 返回值：函数成功返回0；任何其他返回值都表示错误
 */
int pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *cond_attr)
{
    if (!cond)
        return EINVAL;
            
    if (cond_attr) {   /* 修改成参数中的值 */
        memcpy(&cond->cond_attr, cond_attr, sizeof(pthread_mutexattr_t));
    } else {
        cond->cond_attr = __pthread_cond_default_attr;
    }
    pthread_spin_init(&cond->spin, PTHREAD_PROCESS_PRIVATE);
    /* 创建内核的锁的等待队列 */
    cond->waitque = waitque_create();
    if (cond->waitque < 0)
        return ENOMEM;

    return 0;
}
/**
 * pthread_cond_destroy - 销毁条件变量
 * 
 * 返回值：函数成功返回0；任何其他返回值都表示错误
 */
int pthread_cond_destroy(pthread_cond_t *cond)
{
    if (!cond)
        return EINVAL;
    if (cond->waitque >= 0)
        waitque_destroy(cond->waitque);
    cond->waitque = -1;
    return 0;
}

/**
 * pthread_cond_wait - 等待条件变量
 * 
 * 等待条件变量，会先把互斥锁解锁，然后阻塞，被唤醒后再加锁。
 * 解锁到阻塞中间是原子的，唤醒到加锁期间也是原子的。
 * 
 */
int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex)
{
    if (!cond || !mutex)
        return EINVAL;
    /* 如果是通过静态方式创建，那么waitque就需要重新获取 */
    if (cond->waitque == -1) {
        cond->waitque = waitque_create();
        if (cond->waitque < 0)
            return ENOMEM;
    }
    pthread_spin_lock(&cond->spin);    /* 获得对互斥锁得操作 */
    /* 互斥锁解锁 */
    pthread_mutex_unlock(mutex);

    /* 加入等待队列，并把自旋锁解锁 */
    waitque_wait(cond->waitque, (int *) &cond->spin.count, WAITQUE_SET, 0);
    //printf("cond: %d after wait, lock mutex.\n", pthread_self());
    /* 对互斥锁加锁，获得操作。 */
    pthread_mutex_lock(mutex);

    return 0;
}
#if 1
/**
 * pthread_cond_timedwait - 有时间限制的等待条件变量
 * 
 * 功能和pthread_cond_wait一样，只是说，有个时间限制，不会永远等待。
 */
int pthread_cond_timedwait(
    pthread_cond_t *cond,
    pthread_mutex_t *mutex,
    const struct timespec *abstime
) {
    if (!cond || !mutex || !abstime)
        return EINVAL;
    /* 如果是通过静态方式创建，那么waitque就需要重新获取 */
    if (cond->waitque == -1) {
        cond->waitque = waitque_create();
        if (cond->waitque < 0)
            return ENOMEM;
    }
    int retval = 0;
    pthread_spin_lock(&cond->spin);    /* 获得对互斥锁得操作 */
    /* 互斥锁解锁 */
    pthread_mutex_unlock(mutex);

    /* 加入等待队列，并把自旋锁解锁 */
    retval = waitque_wait(cond->waitque, (int *) &cond->spin.count, WAITQUE_TIMED | WAITQUE_ZERO, (unsigned long) abstime);

    //printf("cond: %d after wait, lock mutex.\n", pthread_self());
    /* 对互斥锁加锁，获得操作。 */
    pthread_mutex_lock(mutex);

    return retval;
}
#endif
/**
 * pthread_cond_signal - 发送信号唤醒条件中的一个线程
 * 
 * 
 */
int pthread_cond_signal(pthread_cond_t *cond)
{
    if (!cond)
        return EINVAL;
    /* 如果是通过静态方式创建，那么waitque就需要重新获取 */
    if (cond->waitque == -1) {
        cond->waitque = waitque_create();
        if (cond->waitque < 0)
            return ENOMEM;
    }

    pthread_spin_lock(&cond->spin);    /* 获得对互斥锁得操作 */

    /* 加入等待队列，并把自旋锁解锁 */
    waitque_wake(cond->waitque, NULL, 0, 0);

    pthread_spin_unlock(&cond->spin);    /* 获得对互斥锁得操作 */

    return 0;
}

/**
 * pthread_cond_broadcast - 发送信号唤醒条件中的所有线程
 * 
 * 
 */
int pthread_cond_broadcast(pthread_cond_t *cond)
{
    if (!cond)
        return EINVAL;
    /* 如果是通过静态方式创建，那么waitque就需要重新获取 */
    if (cond->waitque == -1) {
        cond->waitque = waitque_create();
        if (cond->waitque < 0)
            return ENOMEM;
    }
    
    pthread_spin_lock(&cond->spin);    /* 获得对互斥锁得操作 */

    /* 加入等待队列，并把自旋锁解锁 */
    waitque_wake(cond->waitque, NULL, WAITQUE_ALL, 0);

    pthread_spin_unlock(&cond->spin);    /* 获得对互斥锁得操作 */

    return 0;
}

/**
 * pthread_condattr_init - 初始化条件变量属性
 * @attr: 属性值
 * 
 * 返回值：函数成功返回0；任何其他返回值都表示错误
 */
int pthread_condattr_init(pthread_condattr_t *attr)
{
    if (!attr)
        return EINVAL;
    *attr = (pthread_condattr_t) PTHREAD_COND_ATTR_INITIALIZER;
    return 0;
}

/**
 * pthread_condattr_destroy - 销毁条件变量属性
 * @attr: 属性值
 * 
 * 返回值：函数成功返回0；任何其他返回值都表示错误
 */
int pthread_condattr_destroy(pthread_condattr_t *attr)
{
    if (!attr)
        return EINVAL;
    attr->pshared = 0;
    return 0;
}

/**
 * pthread_condattr_getpshared - 获取共享属性
 * @attr: 属性
 * @pshared: 共享属性储存地址
 * 
 * 成功返回0，失败返回非0
 */
int pthread_condattr_getpshared(pthread_condattr_t *attr, int *pshared)
{
    if (!attr || !pshared)
        return EINVAL;
    *pshared = attr->pshared;
    return 0;
}

int pthread_condattr_setpshared(pthread_condattr_t *attr, int pshared)
{
    if (!attr)
        return EINVAL;
    attr->pshared = pshared;
    return 0;
}