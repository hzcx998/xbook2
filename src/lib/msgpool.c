#include <xbook/msgpool.h>
#include <xbook/memalloc.h>
#include <xbook/schedule.h>
#include <string.h>

msgpool_t *msgpool_create(size_t msgsz, size_t msgcount)
{
    if (!msgsz || !msgcount)
        return NULL;
    msgpool_t *pool = mem_alloc(sizeof(msgpool_t));
    if (pool == NULL)
        return NULL;
    pool->msgcount  = 0;
    pool->msgsz     = msgsz;
    pool->msgmaxcnt = msgcount;
    pool->msgbuf    = mem_alloc(msgcount * msgsz);
    if (pool->msgbuf == NULL)
        return NULL;
    memset(pool->msgbuf, 0, msgcount * msgsz);
    mutexlock_init(&pool->mutex);
    wait_queue_init(&pool->waiters);
    pool->tail = pool->head = pool->msgbuf;
    return pool;
}

int msgpool_destroy(msgpool_t *pool)
{
    if (!pool)
        return -1;
    if (wait_queue_length(&pool->waiters) > 0) {
        wait_queue_wakeup_all(&pool->waiters);
    }
    pool->msgmaxcnt = 0;
    pool->msgcount     = 0;
    pool->msgsz     = 0;
    mem_free(pool->msgbuf);
    pool->tail = pool->head = pool->msgbuf = NULL;
    mem_free(pool);
    return 0;
}

int msgpool_put(msgpool_t *pool, void *buf)
{
    if (!pool || !buf)
        return -1;
        
    mutex_lock(&pool->mutex);
    if (msgpool_full(pool)) {
        wait_queue_add(&pool->waiters, task_current);
        
        mutex_unlock(&pool->mutex);
        task_block(TASK_BLOCKED);
        mutex_lock(&pool->mutex);
    }
    memcpy(pool->head, buf, pool->msgsz);   /* copy data */
    pool->head += pool->msgsz;
    /* fix out of boundary */
    if (pool->head >= pool->msgbuf + pool->msgmaxcnt * pool->msgsz)
        pool->head = pool->msgbuf;
    pool->msgcount++;
    if (wait_queue_length(&pool->waiters) > 0)
        wait_queue_wakeup(&pool->waiters);     /* wake up */
    mutex_unlock(&pool->mutex);
    return 0;
}

void *msgpool_put_buf(msgpool_t *pool)
{
    if (!pool)
        return NULL;
    mutex_lock(&pool->mutex);
    if (msgpool_full(pool)) {
        wait_queue_add(&pool->waiters, task_current);
        mutex_unlock(&pool->mutex);
        task_block(TASK_BLOCKED);
        mutex_lock(&pool->mutex);
    }
    return (void *) pool->head;
}

int msgpool_put_buf_sync(msgpool_t *pool)
{
    if (!pool)
        return -1;
    pool->head += pool->msgsz;
    /* fix out of boundary */
    if (pool->head >= pool->msgbuf + pool->msgmaxcnt * pool->msgsz)
        pool->head = pool->msgbuf;
    pool->msgcount++;
    if (wait_queue_length(&pool->waiters) > 0)
        wait_queue_wakeup(&pool->waiters);     /* wake up */
    mutex_unlock(&pool->mutex);
    return 0;
}

int msgpool_try_put(msgpool_t *pool, void *buf)
{
    if (!pool)
        return -1;
    
    mutex_lock(&pool->mutex);
    if (msgpool_full(pool)) {
        mutex_unlock(&pool->mutex);
        return -1;
    }
        
    memcpy(pool->head, buf, pool->msgsz);   /* copy data */
    pool->head += pool->msgsz;
    /* fix out of boundary */
    if (pool->head >= pool->msgbuf + pool->msgmaxcnt * pool->msgsz)
        pool->head = pool->msgbuf;
    pool->msgcount++;
    if (wait_queue_length(&pool->waiters) > 0)
        wait_queue_wakeup(&pool->waiters);     /* wake up */
    mutex_unlock(&pool->mutex);
    return 0;
}

int msgpool_get(msgpool_t *pool, void *buf)
{
    if (!pool)
        return -1;
    mutex_lock(&pool->mutex);
    if (msgpool_empty(pool)) {
        wait_queue_add(&pool->waiters, task_current);
        mutex_unlock(&pool->mutex);
        task_block(TASK_BLOCKED);
        mutex_lock(&pool->mutex);
    }
    if (buf) { /* 有buf才复制 */
        memcpy(buf, pool->tail, pool->msgsz);   /* copy data */
    }
    pool->tail += pool->msgsz;
    /* fix out of boundary */
    if (pool->tail >= pool->msgbuf + pool->msgmaxcnt * pool->msgsz)
        pool->tail = pool->msgbuf;
    pool->msgcount--;
    if (wait_queue_length(&pool->waiters) > 0)
        wait_queue_wakeup(&pool->waiters);     /* wake up */    

    mutex_unlock(&pool->mutex);
    return 0;
}

void *msgpool_get_buf(msgpool_t *pool)
{
    if (!pool)
        return NULL;
    mutex_lock(&pool->mutex);
    if (msgpool_empty(pool)) {
        wait_queue_add(&pool->waiters, task_current);
        mutex_unlock(&pool->mutex);
        task_block(TASK_BLOCKED);
        mutex_lock(&pool->mutex);
    }
    return pool->tail;
}

int msgpool_get_buf_sync(msgpool_t *pool)
{
    if (!pool)
        return -1;
    pool->tail += pool->msgsz;
    /* fix out of boundary */
    if (pool->tail >= pool->msgbuf + pool->msgmaxcnt * pool->msgsz)
        pool->tail = pool->msgbuf;
    pool->msgcount--;
    if (wait_queue_length(&pool->waiters) > 0)
        wait_queue_wakeup(&pool->waiters);     /* wake up */    
    mutex_unlock(&pool->mutex);
    return 0;
}

int msgpool_try_get(msgpool_t *pool, void *buf)
{
    if (!pool)
        return -1;
    mutex_lock(&pool->mutex);
    if (msgpool_empty(pool)) {
        mutex_unlock(&pool->mutex);
        return -1;
    }
    if (buf) { /* 有buf才复制 */
        memcpy(buf, pool->tail, pool->msgsz);   /* copy data */
    }
    pool->tail += pool->msgsz;
    /* fix out of boundary */
    if (pool->tail >= pool->msgbuf + pool->msgmaxcnt * pool->msgsz)
        pool->tail = pool->msgbuf;
    pool->msgcount--;
    if (wait_queue_length(&pool->waiters) > 0)
        wait_queue_wakeup(&pool->waiters);     /* wake up */    

    mutex_unlock(&pool->mutex);
    return 0;
}

int msgpool_empty(msgpool_t *pool)
{
    int empty;
    empty = pool->msgcount <= 0 ? 1: 0;
    return empty;
}

int msgpool_full(msgpool_t *pool)
{
    int full;
    full = pool->msgcount >= pool->msgmaxcnt ? 1: 0;
    return full;
}

int msgpool_count(msgpool_t *pool)
{
    int count;
    count = pool->msgcount;
    return count;
}
