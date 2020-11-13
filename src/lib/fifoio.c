#include <xbook/fifoio.h>
#include <assert.h>
#include <math.h>
#include <string.h>

int fifo_io_init(fifo_io_t *fifo, 
    unsigned char *buffer, unsigned int size)
{
	/* 大小必须是2的n次幂 */
    if (!is_power_of_2(size))
        return -1;
    
    fifo->buffer = buffer;
    fifo->size = size;
    fifo->count = 0;
	memset(fifo->buffer, 0, size);
	fifo->head = fifo->tail = fifo->buffer;
    semaphore_init(&fifo->empty, size);
    semaphore_init(&fifo->full, 0);
    mutexlock_init(&fifo->mutex);
    return 0;
}

fifo_io_t *fifo_io_alloc(unsigned int size)
{
    unsigned char *buffer;
    fifo_io_t *fifo;

    /* 如果size不是2的n次幂，就调整为2的n次幂 */
    if (!is_power_of_2(size)) {
        ASSERT(size < 0x80000000);
        size = roundup_pow_of_two(size);
    }

    buffer = mem_alloc(size);
    if (buffer == NULL)
        return NULL;

    fifo = mem_alloc(sizeof(fifo_io_t));
    if (fifo == NULL) {
        mem_free(buffer);
        return NULL;
    }
    
    fifo_io_init(fifo, buffer, size);
    if (fifo == NULL) {
        mem_free(buffer);
        mem_free(fifo);
    }
    
    return fifo;
}

void fifo_io_free(fifo_io_t *fifo)
{
    mem_free(fifo->buffer);
    mem_free(fifo);
}

void fifo_io_put(fifo_io_t *fifo, unsigned char data)
{
	semaphore_down(&fifo->empty);
    mutex_lock(&fifo->mutex);
    if (fifo_io_avali(fifo) > 0) {
        *fifo->head++ = data;
        if(fifo->head >= fifo->buffer + fifo->size)
            fifo->head = fifo->buffer;
        fifo->count++;    
    }
    mutex_unlock(&fifo->mutex);
    semaphore_up(&fifo->full);
}

unsigned char fifo_io_get(fifo_io_t *fifo)
{
	semaphore_down(&fifo->full);
    mutex_lock(&fifo->mutex);
    unsigned char data = 0;
    if (fifo_io_len(fifo) > 0) {
        data = *fifo->tail++;
        if(fifo->tail >= fifo->buffer + fifo->size)
            fifo->tail = fifo->buffer;    
        fifo->count--;
    }    
    mutex_unlock(&fifo->mutex);
    semaphore_up(&fifo->empty);
	return data;
}
