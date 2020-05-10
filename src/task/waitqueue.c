#include <xbook/waitqueue.h>
#include <xbook/task.h>

/**
 * wait_queue_add - 把进程添加到等待队列中
 * @wait_queue: 等待队列
 * @task: 要添加的任务
 */
void wait_queue_add(wait_queue_t *wait_queue, void *_task)
{
    task_t *task = (task_t *) _task;
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
void wait_queue_remove(wait_queue_t *wait_queue, void *_task)
{
    task_t *task = (task_t *) _task;
	
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
void wait_queue_wakeup(wait_queue_t *wait_queue)
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

/**
 * wait_queue_wakeup_all - 唤醒等待队列中的全部任务
 * @wait_queue: 等待队列
 */
void wait_queue_wakeup_all(wait_queue_t *wait_queue)
{
	/* 添加到队列时，需要关闭中断 */
    unsigned long flags;
    spin_lock_irqsave(&wait_queue->lock, flags);

    task_t *task, *next;
    list_for_each_owner_safe (task, next, &wait_queue->wait_list, list) {
        /* 从当前队列删除 */
		list_del(&task->list);
		/* 唤醒任务 */
		task_wakeup(task);
    }

    spin_unlock_irqrestore(&wait_queue->lock, flags);
}
