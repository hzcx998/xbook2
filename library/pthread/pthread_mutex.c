#include <pthread.h>
#include <sys/mutexqueue.h>
#include <sys/proc.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr)
{
    if (!mutex) {
        return EINVAL;
    }
    *mutex = (pthread_mutex_t) PTHREAD_MUTEX_INITIALIZER;
    if (mutexattr) {   /* 修改成参数中的值 */
        memcpy(&mutex->mattr, mutexattr, sizeof(pthread_mutexattr_t));
    }
    mutex->kind = mutex->mattr.type;  /* 和属性一致 */
    /* 创建内核的锁的等待队列 */
    mutex->mutex_queue = mutex_queue_alloc();
    if (mutex->mutex_queue < 0)
        return ENOMEM;
    return 0;
}
int pthread_mutex_destroy(pthread_mutex_t *mutex)
{
    if (!mutex)
        return EINVAL;
    mutex->count = 0;
    mutex->owner = 0;
    mutex->kind = 0;
    if (mutex->mutex_queue >= 0)
        mutex_queue_free(mutex->mutex_queue);
    mutex->mutex_queue = -1;
    mutex->lock = 0;
    pthread_spin_init(&mutex->spin, PTHREAD_PROCESS_PRIVATE);
    memset(&mutex->mattr, 0, sizeof(pthread_mutexattr_t));
    return 0;
}
int pthread_mutex_lock(pthread_mutex_t *mutex)
{
    if (!mutex)
        return EINVAL;
    /* 如果是通过静态方式创建，那么mutex_queue就需要重新获取 */
    if (mutex->mutex_queue == -1) {
        mutex->mutex_queue = mutex_queue_alloc();
        if (mutex->mutex_queue < 0)
            return ENOMEM;
    }
    
    if (mutex->kind == PTHREAD_MUTEX_NORMAL) {  /* 普通锁 */
        while (1) { /* 用循环避免“惊群”效应 */        
            pthread_spin_lock(&mutex->spin);    /* 获得对互斥锁得操作 */
            /* 如果0，那么说明该互斥锁已经被其它线程持有，自己等待 */
            if (!test_and_set(&mutex->lock, 0)) {
                /*
                在即将阻塞前，需要释放mutex->spin自旋锁。但是释放后到mutex_queue_wait之前的这一部分
                空间是有可能切换到其它线程，其它线程就可能会同时到达这里。那么就违背了准则。
                从获取锁到阻塞期间必须是原子的。
                因此，这里不进行pthread_spin_unlock(&mutex->spin);
                而是通过mutex_queue_wait，来对自旋锁的值进行操作，来修改自旋锁的值，实现解锁。
                因为mutex_queue_wait的执行是原子的，所以只有当他调度出去后，才会产生调度。
                */
                //printf("had locked!\n");
                /* 设置自选锁值为0，并且阻塞自己 */
                mutex_queue_wait(mutex->mutex_queue, (void *) &mutex->spin, MUTEX_QUEUE_ZERO, 0);
            } else {
                mutex->lock = 0;    /* 获得锁 */
                mutex->owner = pthread_self();
                pthread_spin_unlock(&mutex->spin);    /* 取消对互斥锁得操作 */
                break;
            }
        }
    } else if  (mutex->kind == PTHREAD_MUTEX_ERRORCHECK) {  /* 检查锁 */
        while (1) { /* 用循环避免“惊群”效应 */      
            pthread_spin_lock(&mutex->spin);    /* 获得对互斥锁得操作 */

            if (mutex->owner > 0) {   /* 已经有线程持有锁 */
                if (mutex->owner == pthread_self()) {   /* 重复获取锁 */
                    pthread_spin_unlock(&mutex->spin);    /* 取消对互斥锁得操作 */
                    return EDEADLK;
                } else {    /* 获取其它线程的锁 */
                    pthread_spin_unlock(&mutex->spin);    /* 取消对互斥锁得操作 */
                    return EPERM;
                }
            }
            
            /* 如果非0，那么说明该互斥锁已经被其它线程持有，自己等待 */
            if (!test_and_set(&mutex->lock, 0)) {
                /*
                在即将阻塞前，需要释放mutex->spin自旋锁。但是释放后到mutex_queue_wait之前的这一部分
                空间是有可能切换到其它线程，其它线程就可能会同时到达这里。那么就违背了准则。
                从获取锁到阻塞期间必须是原子的。
                因此，这里不进行pthread_spin_unlock(&mutex->spin);
                而是通过mutex_queue_wait，来对自旋锁的值进行操作，来修改自旋锁的值，实现解锁。
                因为mutex_queue_wait的执行是原子的，所以只有当他调度出去后，才会产生调度。
                */
                //printf("had locked!\n");
                /* 设置自选锁值为0，并且阻塞自己 */
                mutex_queue_wait(mutex->mutex_queue, (void *) &mutex->spin, MUTEX_QUEUE_ZERO, 0);
            } else {
                mutex->lock = 0;    /* 获得锁 */
                mutex->owner = pthread_self();
                pthread_spin_unlock(&mutex->spin);    /* 取消对互斥锁得操作 */
                break;
            }
        }
    } else if (mutex->kind == PTHREAD_MUTEX_RECURSIVE) {  /* 可重入锁 */
        while (1) { /* 用循环避免“惊群”效应 */      
            pthread_spin_lock(&mutex->spin);    /* 获得对互斥锁得操作 */

            if (mutex->owner > 0) {   /* 已经有线程持有锁 */
                if (mutex->owner == pthread_self()) {   /* 重复获取锁 */
                    mutex->count++; /* 重入次数增加 */
                    pthread_spin_unlock(&mutex->spin);    /* 取消对互斥锁得操作 */
                    return 0;
                } else {    /* 获取其它线程的锁 */
                    pthread_spin_unlock(&mutex->spin);    /* 取消对互斥锁得操作 */
                    return EPERM;
                }
            }
            /* 第一次获取时，count必须为0 */
            if (mutex->count > 0) {
                pthread_spin_unlock(&mutex->spin);    /* 取消对互斥锁得操作 */
                return EDEADLK;
            }
            
            /* 如果非0，那么说明该互斥锁已经被其它线程持有，自己等待 */
            if (!test_and_set(&mutex->lock, 0)) {
                /*
                在即将阻塞前，需要释放mutex->spin自旋锁。但是释放后到mutex_queue_wait之前的这一部分
                空间是有可能切换到其它线程，其它线程就可能会同时到达这里。那么就违背了准则。
                从获取锁到阻塞期间必须是原子的。
                因此，这里不进行pthread_spin_unlock(&mutex->spin);
                而是通过mutex_queue_wait，来对自旋锁的值进行操作，来修改自旋锁的值，实现解锁。
                因为mutex_queue_wait的执行是原子的，所以只有当他调度出去后，才会产生调度。
                */
                //printf("had locked!\n");
                /* 设置自选锁值为0，并且阻塞自己 */
                mutex_queue_wait(mutex->mutex_queue, (void *) &mutex->spin, MUTEX_QUEUE_ZERO, 0);
            } else {
                mutex->lock = 0;    /* 获得锁 */
                mutex->owner = pthread_self();
                pthread_spin_unlock(&mutex->spin);    /* 取消对互斥锁得操作 */
                break;
            }
        }
    }
    return 0;
}
int pthread_mutex_unlock(pthread_mutex_t *mutex)
{
    if (!mutex)
        return EINVAL;
    
    /* 如果是通过静态方式创建，那么mutex_queue就需要重新获取 */
    if (mutex->mutex_queue == -1) {
        mutex->mutex_queue = mutex_queue_alloc();
        if (mutex->mutex_queue < 0)
            return ENOMEM;
    }

    if (mutex->kind == PTHREAD_MUTEX_NORMAL) {  /* 普通锁 */
        pthread_spin_lock(&mutex->spin);    /* 获得对互斥锁的操作 */
        //printf("[unlock] %d\n", pthread_self());
        
        mutex->lock = 1;    /* 释放锁 */
        mutex->owner = 0;
        mutex_queue_wake(mutex->mutex_queue, NULL, 0, 0);
        pthread_spin_unlock(&mutex->spin);    /* 取消对互斥锁得操作 */
    } else if (mutex->kind == PTHREAD_MUTEX_ERRORCHECK) {  /* 检测锁 */ 
        pthread_spin_lock(&mutex->spin);    /* 获得对互斥锁的操作 */
        if (mutex->lock == 1) {    /* 尝试解锁已经解锁的锁 */
            pthread_spin_unlock(&mutex->spin);    /* 取消对互斥锁得操作 */
            return EPERM;   /* 不允许访问 */
        }

        mutex->lock = 1;    /* 释放锁 */
        mutex->owner = 0;
        mutex_queue_wake(mutex->mutex_queue, NULL, 0, 0);
        pthread_spin_unlock(&mutex->spin);    /* 取消对互斥锁得操作 */
        
    } else if (mutex->kind == PTHREAD_MUTEX_RECURSIVE) {  /* 可重入锁 */ 
        pthread_spin_lock(&mutex->spin);    /* 获得对互斥锁的操作 */
        if (mutex->lock == 1) {    /* 尝试解锁已经解锁的锁 */
            pthread_spin_unlock(&mutex->spin);    /* 取消对互斥锁得操作 */
            return EPERM;   /* 不允许访问 */
        }
        if (mutex->owner > 0) {   /* 已经有线程持有锁 */
            if (mutex->owner == pthread_self()) {   /* 重复释放锁 */
                if (mutex->count > 0) { /* 重入次数大于0，需要自己释放锁 */
                    mutex->count--;     /* 减少重入次数 */
                    pthread_spin_unlock(&mutex->spin);    /* 取消对互斥锁得操作 */
                    return 0;   /* 解锁成功 */
                }
                /* 如果count = 0，那么就会到后面去真正释放锁 */
            }
        }

        mutex->lock = 1;    /* 释放锁 */
        mutex->owner = 0;
        mutex_queue_wake(mutex->mutex_queue, NULL, 0, 0);
        pthread_spin_unlock(&mutex->spin);    /* 取消对互斥锁得操作 */
    } 
    return 0;
}
int pthread_mutex_trylock(pthread_mutex_t *mutex)
{
    if (!mutex)
        return EINVAL;
    
    if (mutex->kind == PTHREAD_MUTEX_NORMAL) {  /* 普通锁 */ 
        /* 可以获取 */
        pthread_spin_lock(&mutex->spin);    /* 获得对互斥锁得操作 */
        if (!test_and_set(&mutex->lock, 0)) {    /* 非1，表示不能被获取，直接返回 */
            pthread_spin_unlock(&mutex->spin);    /* 取消对互斥锁得操作 */
            return EBUSY;
        }
        mutex->lock = 0;   /* 锁值减少 */
        mutex->owner = pthread_self();
        pthread_spin_unlock(&mutex->spin);    /* 取消对互斥锁得操作 */
    } else if (mutex->kind == PTHREAD_MUTEX_ERRORCHECK) {  /* 检测锁 */ 
        /* 可以获取 */
        pthread_spin_lock(&mutex->spin);    /* 获得对互斥锁得操作 */

        if (mutex->owner > 0) {   /* 已经有线程持有锁 */
            if (mutex->owner == pthread_self()) {   /* 重复获取锁 */
                pthread_spin_unlock(&mutex->spin);    /* 取消对互斥锁得操作 */
                return EDEADLK;
            } else {    /* 获取其它线程的锁 */
                pthread_spin_unlock(&mutex->spin);    /* 取消对互斥锁得操作 */
                return EPERM;
            }
        }
        /* 可以获取 */
        pthread_spin_lock(&mutex->spin);    /* 获得对互斥锁得操作 */
        if (!test_and_set(&mutex->lock, 0)) {    /* 非1，表示不能被获取，直接返回 */
            pthread_spin_unlock(&mutex->spin);    /* 取消对互斥锁得操作 */
            return EBUSY;
        }
        mutex->owner = pthread_self(); /* 更新占有者 */
        mutex->lock = 0;   /* 锁值减少 */
        pthread_spin_unlock(&mutex->spin);    /* 取消对互斥锁得操作 */
    } else if (mutex->kind == PTHREAD_MUTEX_RECURSIVE) {  /* 可重入锁 */ 
        /* 可以获取 */
        pthread_spin_lock(&mutex->spin);    /* 获得对互斥锁得操作 */
        if (mutex->owner > 0) {   /* 已经有线程持有锁 */
            if (mutex->owner == pthread_self()) {   /* 重复获取锁 */
                mutex->count++; /* 重入次数增加 */
                pthread_spin_unlock(&mutex->spin);    /* 取消对互斥锁得操作 */
                return 0;
            } else {    /* 获取其它线程的锁 */
                pthread_spin_unlock(&mutex->spin);    /* 取消对互斥锁得操作 */
                return EPERM;
            }
        }
        /* 可以获取 */
        pthread_spin_lock(&mutex->spin);    /* 获得对互斥锁得操作 */
        if (!test_and_set(&mutex->lock, 0)) {     /* 非1，表示不能被获取，直接返回 */
            pthread_spin_unlock(&mutex->spin);    /* 取消对互斥锁得操作 */
            return EBUSY;
        }
        mutex->owner = pthread_self(); /* 更新占有者 */
        mutex->lock = 0;   /* 锁值减少 */
        pthread_spin_unlock(&mutex->spin);    /* 取消对互斥锁得操作 */
    }

    return 0;
}

/**
 * pthread_mutexattr_init - 初始化互斥锁属性
 * @mattr: 互斥锁
 * 
 * 成功完成之后会返回零。其他任何返回值都表示出现了错误。如果出现以下情况，该函数将失败并返回对应的值。
 * ENOMEM 描述: 内存不足，无法初始化互斥锁属性对象。
*/
int pthread_mutexattr_init(pthread_mutexattr_t *mattr)
{
    if (!mattr)
        return EINVAL;
    
    mattr->pshared = PTHREAD_PROCESS_PRIVATE;
    mattr->type = PTHREAD_MUTEX_DEFAULT;
    return 0;
}
int pthread_mutexattr_destroy(pthread_mutexattr_t *mattr)
{
    if (!mattr)
        return EINVAL;
    mattr->pshared = 0;
    mattr->type = 0;
    return 0;
}
int pthread_mutexattr_setpshared(pthread_mutexattr_t *mattr, int pshared)
{
    if (!mattr)
        return EINVAL;
    mattr->pshared = pshared;
    return 0;
}
int pthread_mutexattr_getpshared(pthread_mutexattr_t *mattr, int *pshared)
{
    if (!mattr)
        return EINVAL;
    if (pshared)
        *pshared = mattr->pshared;
    return 0;
}
int pthread_mutexattr_settype(pthread_mutexattr_t *mattr , int type)
{   
    if (!mattr)
        return EINVAL;
    mattr->type = type;
    return 0;
}
int pthread_mutexattr_gettype(pthread_mutexattr_t *mattr , int *type)
{
    if (!mattr)
        return EINVAL;
    if (type)
        *type = mattr->type;
    return 0;
}