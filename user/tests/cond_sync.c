#include"cond_sync.h"
#include<sys/time.h>
#include<errno.h>
 
 
int cond_sync_init(Cond_sync_t* cond_s)
{
    int status;
    status = pthread_mutex_init(&cond_s->mutex, NULL);
    if( status != 0 )
        goto error;
 
    status = pthread_cond_init(&cond_s->cond, NULL);
    if( status != 0 )
    {
        pthread_mutex_destroy(&cond_s->mutex);
        goto error;
    }
    cond_s->sig_num = 0;
    cond_s->valid = COND_SYNC_VALID;
 
    return 0;
 
    error:
        return status;
}
 
 
int cond_sync_destroy(Cond_sync_t* cond_s)
{
    int status1, status2;
 
    if( cond_s == NULL || cond_s->valid != COND_SYNC_VALID )
        return EINVAL;
 
    cond_s->valid = 0;
    //try best to destroy all object, so judge the status' after all
    //object destroy
    status1 = pthread_mutex_destroy(&cond_s->mutex);
    status2 = pthread_cond_destroy(&cond_s->cond);
 
    if( status1 != 0 )
        return status1;
 
    return status2;
}
 
 
void cleanup_unlock(void* arg)
{
    pthread_mutex_t *mutex = (pthread_mutex_t*)arg;
    pthread_mutex_unlock(mutex);
}
 
 
int cond_sync_wait(Cond_sync_t* cond_s)
{
    int status;
 
    if( cond_s == NULL || cond_s->valid != COND_SYNC_VALID )
        return EINVAL;
 
    status = pthread_mutex_lock(&cond_s->mutex);
    if( status != 0 )
        return status;
 
	//因为pthread_cond_wait是可取消点。所以当线程在pthread_cond_wait
	//中睡眠的时候，其他线程调用thread_cancel取消这个睡眠的线程时,
	//睡眠的线程将苏醒，然后继续锁住mutex, 之后就退出终止。
	//所以，要设定一个清理函数，发生这种情况时，在清理函数中解锁。
    pthread_cleanup_push(cleanup_unlock, &cond_s->mutex);
 
    while( cond_s->sig_num <= 0)
    {
        //cann't be interruptted by a signal
        status = pthread_cond_wait(&cond_s->cond, &cond_s->mutex);
        if( status != 0 )
        {
            break;
        }
    }
 
    --cond_s->sig_num; //可用资源减一
 
    pthread_cleanup_pop(0);
 
	//ignore the error. if status == 0 and unlock return not 0.
	//we cann't return this message to user. it will confuse the user
	//the signal is sucessful, but return error code
    pthread_mutex_unlock(&cond_s->mutex); 
 
	return status;
}
 
 
 
int cond_sync_timedwait(Cond_sync_t* cond_s, int msecs) //Millisecond
{
    struct timeval now;
    struct timespec waittime;
    int status;
    int sec;
 
    if( cond_s == NULL || cond_s->valid != COND_SYNC_VALID )
        return EINVAL;
 
    if( msecs < 0 )
        msecs = 0;
 
    sec = msecs / 1000;
    gettimeofday(&now, NULL);
    waittime.tv_sec = now.tv_sec + sec;
    waittime.tv_nsec = (now.tv_usec + (msecs%1000)*1000)*1000;
 
 
    status = pthread_mutex_lock(&cond_s->mutex);
    if( status != 0 )
        return status;
 
    pthread_cleanup_push(cleanup_unlock, &cond_s->mutex);
 
    while( cond_s->sig_num <= 0 )
    {
        status = pthread_cond_timedwait(&cond_s->cond, &cond_s->mutex, &waittime);
        if( status == EINTR ) //can be interruptted by a signal.
            continue;
        else
            break;
    }
 
    if( status == 0 )
        --cond_s->sig_num;
 
    pthread_cleanup_pop(0);
 
	//ignore the error. if status == 0 and unlock return not 0.
	//we cann't return this message to user. it will confuse the user
	//the signal is sucessful, but return error code
    pthread_mutex_unlock(&cond_s->mutex);
 
	return status;
}
 
 
int cond_sync_signal(Cond_sync_t* cond_s)
{
    int status;
 
    if( cond_s == NULL || cond_s->valid != COND_SYNC_VALID )
        return EINVAL;
 
    status = pthread_mutex_lock(&cond_s->mutex);
    if( status != 0 )
        goto error;
 
    ++cond_s->sig_num; //加一，表示可用资源多了一个
    status = pthread_mutex_unlock(&cond_s->mutex);
    if( status != 0 )
        goto error;
 
    status = pthread_cond_signal(&cond_s->cond);
 
    error:
        return status;
}
