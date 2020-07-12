#include <xbook/fifoio.h>
#include <assert.h>
#include <math.h>

/**
 * fifo_io_init - io队列的初始化
 * @fifo: io队列
 */
int fifo_io_init(fifo_io_t *fifo, 
    unsigned char *buffer, unsigned int size)
{
	/* 大小必须是2的n次幂 */
    if (!is_power_of_2(size))
        return -1;
    
    fifo->buffer = buffer;

    /* 数据区以字节为单位的长度 */
    fifo->size = size;
    
	memset(fifo->buffer, 0, size);

	/* 把头指针和尾指针都指向buf */
	fifo->head = fifo->tail = fifo->buffer;
	/* 空信号量数和缓冲区一样长 */
    semaphore_init(&fifo->empty, size);
    semaphore_init(&fifo->full, 0);
    mutexlock_init(&fifo->mutex);
    return 0;
}

/**
 * fifo_io_alloc - 动态创建一个io队列
 */
fifo_io_t *fifo_io_alloc(unsigned int size)
{
    unsigned char *buffer;
    fifo_io_t *fifo;

    /* 如果size不是2的n次幂，就调整为2的n次幂 */
    if (!is_power_of_2(size)) {
        ASSERT(size < 0x80000000);
        size = roundup_pow_of_two(size);
    }

    buffer = kmalloc(size);
    if (buffer == NULL)
        return NULL;

    fifo = kmalloc(sizeof(fifo_io_t));
    if (fifo == NULL) {
        kfree(buffer);
        return NULL;
    }
    
    fifo_io_init(fifo, buffer, size);
    if (fifo == NULL) {
        kfree(buffer);
        kfree(fifo);
    }
    
    return fifo;
}

void fifo_io_free(fifo_io_t *fifo)
{
    kfree(fifo->buffer);
    kfree(fifo);
}

/**
 * fifo_io_put - 往io队列中放入一个数据
 * @fifo: io队列
 * @data: 数据
 */
void fifo_io_put(fifo_io_t *fifo, unsigned char data)
{
	semaphore_down(&fifo->empty); /* 获取空信号，表示要存放数据 */
    mutex_lock(&fifo->mutex);   /* 互斥锁上锁，保护数据存放 */

    /* 放入一个数据 */
    *fifo->head++ = data;
	if(fifo->head >= fifo->buffer + fifo->size) /* 修复越界 */
		fifo->head = fifo->buffer;

    mutex_unlock(&fifo->mutex);   /* 互斥锁解锁 */
    semaphore_up(&fifo->full); /* 释放满信号，表示有1个数据可以获取 */
}

/**
 * fifo_io_get - 从io队列中获取一个数据
 * @fifo: io队列
 */
unsigned char fifo_io_get(fifo_io_t *fifo)
{
	semaphore_down(&fifo->full); /* 获取满信号，表示要获取一个数据 */
    mutex_lock(&fifo->mutex);   /* 互斥锁上锁，保护数据获取 */
    
    /* 获取一个数据 */
    unsigned char data = *fifo->tail++;
    if(fifo->tail >= fifo->buffer + fifo->size) /* 如果到达了最后面，就跳到最前面，形成一个环 */
		fifo->tail = fifo->buffer;
    
    mutex_unlock(&fifo->mutex);   /* 互斥锁解锁 */
    semaphore_up(&fifo->empty); /* 释放空信号，表示腾出一个数据 */

	return data;
}
