#include <arch/interrupt.h>
#include <xbook/schedule.h>
#include <xbook/debug.h>
#include <xbook/task.h>
#include <xbook/process.h>
#include <xbook/pthread.h>
#include <xbook/waitque.h>
#include <sys/waitque.h>

#define DEBUG_LOCAL 0

/* waitque储存池 */
waitque_t *waitque_table;

/* 访问储存池需要互斥 */
DEFINE_SPIN_LOCK(waitque_table_lock);

/**
 * sys_waitque_create - 创建一个等待队列
 * 
 * 返回一个等待队列句柄
 */
int sys_waitque_create()
{
    waitque_t *waitque = &waitque_table[0];
    int i;
    spin_lock(&waitque_table_lock);
    for (i = 0; i < WAITQUE_NR; i++) {
        if (!waitque->flags) { /* 没有使用 */
            wait_queue_init(&waitque->wait_queue);
            waitque->flags = WAITQUE_USING;
            spin_unlock(&waitque_table_lock);
#if DEBUG_LOCAL == 1
            printk(KERN_DEBUG "%s: pid=%d get handle=%d\n", __func__, current_task->pid, i);
#endif
            return i;
        }
        waitque++;
    }
    spin_unlock(&waitque_table_lock);
#if DEBUG_LOCAL == 1
    printk(KERN_DEBUG "%s: pid=%d get handle failed!\n", __func__, current_task->pid);
#endif
    return -1;
}

/**
 * sys_waitque_destroy - 销毁一个等待队列
 * @handle: 等待队列句柄
 * 
 * 销毁一个等待队列，移除等待队列上面的任务
 */
int sys_waitque_destroy(int handle)
{
    if (IS_BAD_WAITQUE(handle))
        return -1;

    waitque_t *waitque = &waitque_table[handle];
    if (waitque->flags) {
        /* 遍历唤醒等待队列上的任务 */
        while (!list_empty(&waitque->wait_queue.wait_list))
        {
            wait_queue_wakeup(&waitque->wait_queue);
        }
        waitque->flags = 0;
    }
    return 0;
}


/**
 * sys_waitque_wait - 添加到等待队列
 * @handle: 等待队列句柄
 * @addr: 需要操作的数据地址
 * @wqflags: 操作标志：
 *          WAITQUE_SET： 设置addr里面的值为value
 *          WAITQUE_ADD： 往addr里面的值加上value
 *          WAITQUE_SUB： 往addr里面的值减去value
 * @value: 参数变量
 * 
 * 把自己添加到等待队列并阻塞
 * 修改值和阻塞的过程是原子操作
 */
int sys_waitque_wait(int handle, int *addr, int wqflags, int value)
{
    if (IS_BAD_WAITQUE(handle))
        return -1;

    unsigned long flags;
    save_intr(flags);
    waitque_t *waitque = &waitque_table[handle];
    if (waitque->flags) {
#if DEBUG_LOCAL == 1
        printk(KERN_DEBUG "%s: pid=%d wait\n", __func__, current_task->pid);
#endif
        /* 对变量进行操作 */
        if (addr) {
            switch (wqflags)
            {
            case WAITQUE_ADD:
                *addr = *addr + value;
                break;
            case WAITQUE_SUB:
                *addr = *addr - value;
                break;
            case WAITQUE_SET:
                *addr = value;
                break;
            default:
                break;
            }
        }

        wait_queue_add(&waitque->wait_queue, current_task);
        task_block(TASK_BLOCKED);   /* 阻塞 */
        
    }
    restore_intr(flags);
    return 0;
}

/**
 * sys_waitque_wake - 从等待队列唤醒一个任务
 * @handle: 等待队列句柄
 * @addr: 需要操作的数据地址
 * @wqflags: 操作标志：
 *          WAITQUE_SET： 设置addr里面的值为value
 *          WAITQUE_ADD： 往addr里面的值加上value
 *          WAITQUE_SUB： 往addr里面的值减去value
 * @value: 参数变量
 * 
 * 唤醒第一个等待中的任务
 * 修改值和阻塞的过程是原子操作
 */
int sys_waitque_wake(int handle, int *addr, int wqflags, int value)
{
    if (IS_BAD_WAITQUE(handle))
        return -1;
    
    unsigned long flags;
    save_intr(flags);
    waitque_t *waitque = &waitque_table[handle];
    if (waitque->flags) {
#if DEBUG_LOCAL == 1
        printk(KERN_DEBUG "%s: pid=%d wake\n", __func__, current_task->pid);
#endif
        /* 对变量进行操作 */
        if (value) {
            switch (wqflags)
            {
            case WAITQUE_ADD:
                *addr = *addr + value;
                break;
            case WAITQUE_SUB:
                *addr = *addr - value;
                break;
            case WAITQUE_SET:
                *addr = value;
                break;
            default:
                break;
            }
        }
        wait_queue_wakeup(&waitque->wait_queue);
    }
    restore_intr(flags);
    return 0;
}

/**
 * init_waitque - 初始化用户态锁
 * 
 */
void init_waitque()
{
    waitque_table = kmalloc(WAITQUE_NR * sizeof(waitque_t));
    if (waitque_table == NULL)
        panic("%s: alloc memory for waitque table failed!\n", __func__);
    int i;
    for (i = 0; i < WAITQUE_NR; i++) {
        wait_queue_init(&waitque_table[i].wait_queue);
        waitque_table[i].flags = 0;
    }
}
