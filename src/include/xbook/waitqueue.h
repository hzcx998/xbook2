#ifndef _XBOOK_WAIT_QUEUE_H
#define _XBOOK_WAIT_QUEUE_H

#include <xbook/list.h>
#include "debug.h"
#include "spinlock.h"
#include <assert.h>

typedef struct wait_queue {
	list_t wait_list;	// 记录所有被挂起的进程（等待中）的链表
	spinlock_t lock;    /* 维护队列 */
} wait_queue_t;

#define WAIT_QUEUE_INIT(wait_queue) \
    { .wait_list = LIST_HEAD_INIT((wait_queue).wait_list) \
    , .lock = SPIN_LOCK_INIT_UNLOCKED() \
    }

/**
 * wait_queue_init - 等待队列初始化
 * @wait_queue: 等待队列
 */
static inline void wait_queue_init(wait_queue_t *wait_queue)
{
	/* 初始化队列 */
	INIT_LIST_HEAD(&wait_queue->wait_list);
	spinlock_init(&wait_queue->lock);
}

void wait_queue_add(wait_queue_t *wait_queue, void *_task);
void wait_queue_remove(wait_queue_t *wait_queue, void *_task);
void wait_queue_wakeup(wait_queue_t *wait_queue);
void wait_queue_wakeup_all(wait_queue_t *wait_queue);

static inline int wait_queue_length(wait_queue_t *wait_queue)
{
    //spin_lock(&wait_queue->lock);
    int len = list_length(&wait_queue->wait_list);
    //spin_unlock(&wait_queue->lock);
    return len;
}

#endif   /* _XBOOK_WAIT_QUEUE_H */
