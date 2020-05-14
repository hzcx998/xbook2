#ifndef _XBOOK_FIFO_BUF_H
#define _XBOOK_FIFO_BUF_H

#include "spinlock.h"

/* this came from kfifo in linux kernel. */

typedef struct fifo_buf {
    unsigned char *buffer;  /* the buffer holding the data */
    unsigned int size;      /* the size of the allocated buffer */
    unsigned int in;        /* data is added at offset (in % size) */
    unsigned int out;       /* data is extracted from off. (out % size) */
    spinlock_t lock;       /* protects concurrent modifications */
} fifo_buf_t;

#define FIFO_BUF_INIT(fifoname, buf, sz) \
    { .buffer = buf \
    , .size = sz \
    , .in = 0 \
    , .out = 0 \
    , .lock = SPIN_LOCK_INIT_UNLOCKED() \
    }

#define DEFINE_FIFO_BUF(fifoname, buffer, size) \
    fifo_buf_t fifoname = FIFO_BUF_INIT(fifoname, buffer, size)

void fifo_buf_init(fifo_buf_t *fifo, unsigned char *buffer,
        unsigned int size);
fifo_buf_t *fifo_buf_alloc(unsigned int size);
void fifo_buf_free(fifo_buf_t *fifo);

unsigned int __fifo_buf_get(fifo_buf_t *fifo, 
        const unsigned char *buffer, unsigned int len);
unsigned int __fifo_buf_put(fifo_buf_t *fifo, 
        const unsigned char *buffer, unsigned int len);

static inline unsigned int fifo_buf_put(fifo_buf_t *fifo, 
        const unsigned char *buffer, unsigned int len)
{
    unsigned long flags;
    unsigned int ret;
    spin_lock_irqsave(&fifo->lock, flags);
    ret = __fifo_buf_put(fifo, buffer, len);
    spin_unlock_irqrestore(&fifo->lock, flags);
    return ret;
}

static inline unsigned int fifo_buf_get(fifo_buf_t *fifo, 
        const unsigned char *buffer, unsigned int len)
{
    unsigned long flags;
    unsigned int ret;
    spin_lock_irqsave(&fifo->lock, flags);
    ret = __fifo_buf_get(fifo, buffer, len);
    /* 当fifo->in == fifo->out时，buffer为空 */
    if (fifo->in == fifo->out)
        fifo->in = fifo->out = 0;
    spin_unlock_irqrestore(&fifo->lock, flags);
    return ret;
}

static inline void __fifo_buf_reset(fifo_buf_t *fifo)
{
    fifo->in = fifo->out = 0;
}

static inline void fifo_buf_reset(fifo_buf_t *fifo)
{
    unsigned long flags;
    spin_lock_irqsave(&fifo->lock, flags);
    __fifo_buf_reset(fifo);
    spin_unlock_irqrestore(&fifo->lock, flags);
}

static inline unsigned int __fifo_buf_len(fifo_buf_t *fifo)
{
    return fifo->in - fifo->out;
}

static inline unsigned int fifo_buf_len(fifo_buf_t *fifo)
{
    unsigned long flags;
    unsigned int ret;
    spin_lock_irqsave(&fifo->lock, flags);
    ret = __fifo_buf_len(fifo);
    spin_unlock_irqrestore(&fifo->lock, flags);
    return ret;
}

static inline unsigned int __fifo_buf_avali(fifo_buf_t *fifo)
{
    return fifo->size - (fifo->in - fifo->out);
}

static inline unsigned int fifo_buf_avali(fifo_buf_t *fifo)
{
    unsigned long flags;
    unsigned int ret;
    spin_lock_irqsave(&fifo->lock, flags);
    ret = __fifo_buf_avali(fifo);
    spin_unlock_irqrestore(&fifo->lock, flags);
    return ret;
}


#endif /* _XBOOK_FIFO_BUF_H */
