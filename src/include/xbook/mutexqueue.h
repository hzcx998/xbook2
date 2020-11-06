#ifndef _XBOOK_MUTEX_QUEUE_H
#define _XBOOK_MUTEX_QUEUE_H

#include "waitqueue.h"

#define MUTEX_QUEUE_USING           (1 << 0)

typedef struct {
    wait_queue_t wait_queue;
    unsigned int flags;
} mutex_queue_t;

#define MUTEX_QUEUE_NR_MAX       128
#define MUTEX_QUEUE_IS_BAD(handle) \
    ((handle) < 0 || (handle) >= MUTEX_QUEUE_NR_MAX)

void mutex_queue_init();
int sys_mutex_queue_wake(int handle, void *addr, unsigned int wqflags, unsigned long value);
int sys_mutex_queue_wait(int handle, void *addr, unsigned int wqflags, unsigned long value);
int sys_mutex_queue_alloc(int handle);
int sys_mutex_queue_free();

#endif   /* _XBOOK_MUTEX_QUEUE_H */
