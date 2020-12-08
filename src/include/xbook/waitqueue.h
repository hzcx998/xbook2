#ifndef _XBOOK_WAIT_QUEUE_H
#define _XBOOK_WAIT_QUEUE_H

#include <xbook/list.h>
#include "debug.h"
#include "spinlock.h"
#include <assert.h>

typedef struct wait_queue {
	list_t wait_list;	// 记录所有被挂起的进程（等待中）的链表
	spinlock_t lock;
} wait_queue_t;

#define WAIT_QUEUE_INIT(wait_queue) \
    { .wait_list = LIST_HEAD_INIT((wait_queue).wait_list) \
    , .lock = SPIN_LOCK_INIT_UNLOCKED() \
    }

void wait_queue_add(wait_queue_t *wait_queue, void *task_ptr);
void wait_queue_remove(wait_queue_t *wait_queue, void *task_ptr);
void wait_queue_wakeup(wait_queue_t *wait_queue);
void wait_queue_wakeup_all(wait_queue_t *wait_queue);
void wait_queue_sleepon(wait_queue_t *wait_queue);

static inline void wait_queue_init(wait_queue_t *wait_queue)
{
	list_init(&wait_queue->wait_list);
	spinlock_init(&wait_queue->lock);
}
static inline int wait_queue_length(wait_queue_t *wait_queue)
{
    unsigned long flags;
    spin_lock_irqsave(&wait_queue->lock, flags);
    int len = list_length(&wait_queue->wait_list);
    spin_unlock_irqrestore(&wait_queue->lock, flags);
    return len;
}

#endif   /* _XBOOK_WAIT_QUEUE_H */
