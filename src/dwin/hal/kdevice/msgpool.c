#include <dwin/dwin.h>
#include <dwin/hal.h>

/* xbook kernel header */
#include <xbook/msgpool.h>
#include <xbook/memalloc.h>

static struct dwin_msgpool *create(int msgsz, int msgcnt)
{
    struct dwin_msgpool *pool = mem_alloc(sizeof(struct dwin_msgpool));
    if (pool == NULL)
    {
        return NULL;
    }
    pool->object = msgpool_create(msgsz, msgcnt);
    if (pool->object == NULL)
    {
        mem_free(pool);
        return NULL;
    }
    pool->msgsz = msgsz;
    pool->msgcnt = msgcnt;
    return pool;
}

static int destroy(struct dwin_msgpool *msgpool)
{
    if (msgpool->object == NULL)
    {
        return -1;
    }
    if (msgpool_destroy(msgpool->object) < 0)
    {
        return -1;
    }
    mem_free(msgpool);
    return 0;    
}

static int send(struct dwin_msgpool *msgpool, void *buf, int flags)
{
    if (msgpool == NULL || buf == NULL)
    {
        return -1;
    }

    if (flags & DWIN_NOBLOCK)
    {
        if (msgpool_try_put(msgpool->object, buf, msgpool->msgsz) < 0)
        {
            return -1;
        }
    }
    else
    {
        if (msgpool_put(msgpool->object, buf, msgpool->msgsz) < 0)
        {
            return -1;
        }
    }
    return 0;
}

static int recv(struct dwin_msgpool *msgpool, void *buf, int flags)
{
    if (msgpool == NULL || buf == NULL)
    {
        return -1;
    }

    if (flags & DWIN_NOBLOCK)
    {
        if (msgpool_try_get(msgpool->object, buf, NULL) < 0)
        {
            return -1;
        }
    }
    else
    {
        if (msgpool_get(msgpool->object, buf, NULL) < 0)
        {
            return -1;
        }
    }
    return 0;    
}

struct dwin_hal_msgpool __kdevice_msgpool_hal = {
    .create = create,
    .destroy = destroy,
    .send = send,
    .recv = recv,
    .extension = NULL,
};
