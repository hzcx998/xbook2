#include <semaphore.h>
#include <errno.h>
#include <stddef.h>
#include <sys/types.h>
#include <sys/time.h>

int sem_init(sem_t *sem, int pshared, unsigned int value)
{
    if (sem == NULL || value > SEM_VALUE_MAX) {
        return EINVAL;
    }

    if (pshared) {
        return ENOSYS;
    }

    int status;
    status = pthread_mutex_init(&sem->mutex, NULL);
    if(status != 0)
        goto error;
 
    status = pthread_cond_init(&sem->cond, NULL);
    if(status != 0)
    {
        pthread_mutex_destroy(&sem->mutex);
        goto error;
    }
    sem->value = value;
    sem->valid = SEM_VALID;
    return 0;
error:
    return status;
}

int sem_destroy(sem_t *sem)
{
    int status1, status2;
 
    if(sem == NULL || sem->valid != SEM_VALID) {
        return EINVAL;
    }
    sem->value = 0;
    sem->valid = 0;
    //try best to destroy all object, so judge the status' after all
    //object destroy
    status1 = pthread_mutex_destroy(&sem->mutex);
    status2 = pthread_cond_destroy(&sem->cond);
 
    if(status1 != 0)
        return status1;

    return status2;
}

/**
 * returns 0 on success; on error, -1 is returned and errno
 * is set to indicate the error.
 */
int sem_getvalue(sem_t *sem, int *sval)
{
    if (sem == NULL || sval == NULL) {
        return EINVAL;
    }
    *sval = sem->value;
    return 0;
}

static void cleanup_unlock(void* arg)
{
    pthread_mutex_t *mutex = (pthread_mutex_t*)arg;
    pthread_mutex_unlock(mutex);
}

int sem_wait(sem_t *sem)
{
    int status;
    
    if(sem == NULL || sem->valid != SEM_VALID) {
        return EINVAL;
    }
    status = pthread_mutex_lock(&sem->mutex);
    if(status != 0)
        return status;
 
	//因为pthread_cond_wait是可取消点。所以当线程在pthread_cond_wait
	//中睡眠的时候，其他线程调用thread_cancel取消这个睡眠的线程时,
	//睡眠的线程将苏醒，然后继续锁住mutex, 之后就退出终止。
	//所以，要设定一个清理函数，发生这种情况时，在清理函数中解锁。
    pthread_cleanup_push(cleanup_unlock, &sem->mutex);
 
    while(sem->value <= 0) {
        //cann't be interruptted by a signal
        status = pthread_cond_wait(&sem->cond, &sem->mutex);
        if( status != 0 )
            break;
    }
 
    --sem->value; //可用资源减一
 
    pthread_cleanup_pop(0);
 
	//ignore the error. if status == 0 and unlock return not 0.
	//we cann't return this message to user. it will confuse the user
	//the signal is sucessful, but return error code
    pthread_mutex_unlock(&sem->mutex); 
 
	return status;
}

int sem_timedwait(sem_t *sem, const struct timespec *abs_timeout)
{
    int status;

    if(sem == NULL || sem->valid != SEM_VALID) {
        return EINVAL;
    }

    status = pthread_mutex_lock(&sem->mutex);
    if( status != 0 )
        return status;
 
    pthread_cleanup_push(cleanup_unlock, &sem->mutex);
 
    while(sem->value <= 0) {
        status = pthread_cond_timedwait(&sem->cond, &sem->mutex, abs_timeout);
        if( status == EINTR ) //can be interruptted by a signal.
            continue;
        else
            break;
    }
 
    if(status == 0)
        --sem->value;
        
    pthread_cleanup_pop(0);
 
	//ignore the error. if status == 0 and unlock return not 0.
	//we cann't return this message to user. it will confuse the user
	//the signal is sucessful, but return error code
    pthread_mutex_unlock(&sem->mutex);
    
	return status;
}

int sem_trywait(sem_t *sem)
{
    if (sem == NULL) {
        return EINVAL;
    }
    /* 没有超时时间，直接返回 */
    int status = sem_timedwait(sem, NULL);
    if( status == ETIMEDOUT )
        return EAGAIN;
    
    return status;
}

int sem_post(sem_t *sem)
{
    int status;
    if( sem == NULL || sem->valid != SEM_VALID ) {
        return EINVAL;
    }
 
    status = pthread_mutex_lock(&sem->mutex);
    if( status != 0 )
        goto error;
 
    ++sem->value; //加一，表示可用资源多了一个
    status = pthread_mutex_unlock(&sem->mutex);
    if( status != 0 )
        goto error;
 
    status = pthread_cond_signal(&sem->cond);
 
    if (sem->value > SEM_VALUE_MAX)
        return EOVERFLOW;
    
error:
    return status;
}

