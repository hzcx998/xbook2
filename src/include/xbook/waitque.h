#ifndef _XBOOK_WAIT_QUE_H
#define _XBOOK_WAIT_QUE_H

/* 用户态使用的等待队列 */
#include "waitqueue.h"

#define WAITQUE_USING           (1 << 0)

/* 用户态等待队列 */
typedef struct __waitque {
    wait_queue_t wait_queue;    /* 等待队列 */
    unsigned int flags;         /* 标志 */
} waitque_t;

/* 内核支持的用户等待队列数量 */
#define WAITQUE_NR       128

#define IS_BAD_WAITQUE(handle) \
    ((handle) < 0 || (handle) >= WAITQUE_NR)

void init_waitque();
int sys_waitque_wake(int handle, void *addr, unsigned int wqflags, unsigned long value);
int sys_waitque_wait(int handle, void *addr, unsigned int wqflags, unsigned long value);
int sys_waitque_destroy(int handle);
int sys_waitque_create();

#endif   /* _XBOOK_WAIT_QUE_H */
