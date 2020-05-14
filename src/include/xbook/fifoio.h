#ifndef _XBOOK_FIFO_IO_H
#define	_XBOOK_FIFO_IO_H

#include "synclock.h"
#include "task.h"
#include "mutexlock.h"

/*
生产者消费者模型来实现fifoio队列
用信号量的值来表示数据量
*/
typedef struct fifo_io {
    unsigned char *buffer;  /* 缓冲区 */
    unsigned int size;      /* 缓冲区大小 */
    unsigned char *head;	/* 队首,数据往队首处写入 */
    unsigned char *tail;    /* 队尾,数据从队尾处读出 */
	semaphore_t full;       /* 满数据信号量，已经放入数据的数量 */
    semaphore_t empty;      /* 空数据信号量，剩余的数据数量 */
    mutexlock_t mutex;      /* 互斥锁，保证存放数据互斥 */
} fifo_io_t;

#define FIFO_IO_INIT(fifoname, buf, sz) \
    { .buffer = (buf) \
    , .size = (sz) \
    , .head = (buf) \
    , .tail = (buf) \
    , .full = SEMAPHORE_INIT((fifoname).full, 0) \
    , .empty = SEMAPHORE_INIT((fifoname).empty, (sz)) \
    , .mutex = MUTEX_LOCK_INIT((fifoname).mutex) \
    }

#define DEFINE_FIFO_IO(fifoname, buf, size) \
    fifo_io_t fifoname = FIFO_IO_INIT((fifoname), (buf), (size))

fifo_io_t *fifo_io_alloc(unsigned int size);

int fifo_io_init(fifo_io_t *fifo, 
    unsigned char *buffer, unsigned int buflen);

unsigned char fifo_io_get(fifo_io_t *fifo);
void fifo_io_put(fifo_io_t *fifo, unsigned char data); 

static inline bool fifo_io_len(fifo_io_t *fifo)
{
    unsigned int len;
    mutex_lock(&fifo->mutex);
    len = atomic_get(&fifo->full.counter);
    mutex_unlock(&fifo->mutex);
    return len;
}

static inline bool fifo_io_avali(fifo_io_t *fifo)
{
    unsigned int len;
    mutex_lock(&fifo->mutex);
    len = atomic_get(&fifo->empty.counter);
    mutex_unlock(&fifo->mutex);
    return len;
}

#endif /* _XBOOK_FIFO_IO_H */