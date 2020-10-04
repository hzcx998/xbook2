#include <xbook/msgpool.h>
#include <xbook/kmalloc.h>
#include <xbook/schedule.h>
#include <string.h>

msgpool_t *msgpool_create(size_t msgsz, size_t msgcount)
{
    if (!msgsz || !msgcount)
        return NULL;
    msgpool_t *pool = kmalloc(sizeof(msgpool_t));
    if (pool == NULL)
        return NULL;
    pool->msgcount  = 0;
    pool->msgsz     = msgsz;
    pool->msgmaxcnt = msgcount;
    pool->msgbuf    = kmalloc(msgcount * msgsz);
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
    if (msgpool_empty(pool) && wait_queue_length(&pool->waiters) <= 0) {
        pool->msgmaxcnt = 0;
        pool->msgcount     = 0;
        pool->msgsz     = 0;
        kfree(pool->msgbuf);
        pool->tail = pool->head = pool->msgbuf = NULL;
        kfree(pool);
        return 0;
    }
    return -1;
}

int msgpool_push(msgpool_t *pool, void *buf)
{
    if (!pool || !buf)
        return -1;
        
    mutex_lock(&pool->mutex);
    if (msgpool_full(pool)) {
        wait_queue_add(&pool->waiters, current_task);
        
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

int msgpool_try_push(msgpool_t *pool, void *buf)
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

int msgpool_pop(msgpool_t *pool, void *buf)
{
    if (!pool)
        return -1;
    mutex_lock(&pool->mutex);
    if (msgpool_empty(pool)) {
        wait_queue_add(&pool->waiters, current_task);
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

int msgpool_try_pop(msgpool_t *pool, void *buf)
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
