#ifndef _XBOOK_WAIT_QUEUE_H
#define _XBOOK_WAIT_QUEUE_H

#include <xbook/list.h>
#include <xbook/task.h>
#include <xbook/debug.h>
#include <xbook/spinlock.h>
#include <xbook/assert.h>

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

/**
 * wait_queue_add - 把进程添加到等待队列中
 * @wait_queue: 等待队列
 * @task: 要添加的任务
 */
static inline void wait_queue_add(wait_queue_t *wait_queue, task_t *task)
{
	/* 添加到队列时，需要关闭中断 */
    unsigned long flags;
    spin_lock_irqsave(&wait_queue->lock, flags);

	/* 确保任务不在等待队列中 */
	ASSERT(!list_find(&task->list, &wait_queue->wait_list));
	
	/* 添加到等待队列中，添加到最后 */
	list_add_tail(&task->list, &wait_queue->wait_list);

    spin_unlock_irqrestore(&wait_queue->lock, flags);
}

/**
 * wait_queue_remove - 把进程从等待队列中移除
 * @wait_queue: 等待队列
 * @task: 要移除的任务
 */
static inline void wait_queue_remove(wait_queue_t *wait_queue, task_t *task)
{
	/* 添加到队列时，需要关闭中断 */
    unsigned long flags;
    spin_lock_irqsave(&wait_queue->lock, flags);

	task_t *target, *next;
	/* 在队列中寻找任务，找到后就把任务从队列中删除 */
	list_for_each_owner_safe (target, next, &wait_queue->wait_list, list) {
		if (target == task) {
			/* 把任务从等待队列中删除 */
			list_del_init(&target->list);
			break;
		}
	}

    spin_unlock_irqrestore(&wait_queue->lock, flags);
}

/**
 * wait_queue_wakeup - 唤醒等待队列中的一个任务
 * @wait_queue: 等待队列
 */
static inline void wait_queue_wakeup(wait_queue_t *wait_queue)
{
	/* 添加到队列时，需要关闭中断 */
    unsigned long flags;
    spin_lock_irqsave(&wait_queue->lock, flags);

	/* 不是空队列就获取第一个等待者 */
	if (!list_empty(&wait_queue->wait_list)) {
		/* 获取任务 */		
        task_t *task = list_first_owner(&wait_queue->wait_list, task_t, list);
		
        /* 从当前队列删除 */
		list_del(&task->list);
		/* 唤醒任务 */
		task_wakeup(task);
    }
    
    spin_unlock_irqrestore(&wait_queue->lock, flags);
}

#endif   /* _XBOOK_WAIT_QUEUE_H */
