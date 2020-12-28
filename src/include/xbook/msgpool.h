#ifndef _XBOOK_MSGPOOL_H
#define _XBOOK_MSGPOOL_H

#include <stddef.h>
#include <stdint.h>
#include <xbook/mutexlock.h>
#include <xbook/waitqueue.h>

typedef struct {
    size_t msgsz;       /* message size */
    size_t msgcount;    /* message count */
    size_t msgmaxcnt;   /* message max count */
    uint8_t *head;      /* message head */
    uint8_t *tail;      /* message tail */
    uint8_t *msgbuf;    /* message buf */
    mutexlock_t mutex;  /* message mutex */
    wait_queue_t waiters;   /* message waiters */
} msgpool_t;

typedef void (*msgpool_get_func_t)(msgpool_t *, void *);

msgpool_t *msgpool_create(size_t msgsz, size_t msgcount);
int msgpool_destroy(msgpool_t *pool);
int msgpool_put(msgpool_t *pool, void *buf, size_t size);
int msgpool_get(msgpool_t *pool, void *buf, msgpool_get_func_t callback);
int msgpool_try_put(msgpool_t *pool, void *buf, size_t size);
int msgpool_try_get(msgpool_t *pool, void *buf, msgpool_get_func_t callback);

int msgpool_empty(msgpool_t *pool);
int msgpool_full(msgpool_t *pool);
int msgpool_count(msgpool_t *pool);

#endif /* _XBOOK_MSGPOOL_H */
