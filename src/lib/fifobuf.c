#include <xbook/fifobuf.h>
#include <xbook/memalloc.h>
#include <arch/memory.h>
#include <math.h>
#include <assert.h>
#include <string.h>

void fifo_buf_init(fifo_buf_t *fifo, unsigned char *buffer,
        unsigned int size)
{
    /* 大小必须是2的n次幂 */
    if (!is_power_of_2(size))
        return;
    
    fifo->buffer = buffer;
    fifo->size = size;
    fifo->in = fifo->out = 0;
    spinlock_init(&fifo->lock);
}

fifo_buf_t *fifo_buf_alloc(unsigned int size)
{
    unsigned char *buffer;
    fifo_buf_t *fifo;

    /* 如果size不是2的n次幂，就调整为2的n次幂 */
    if (!is_power_of_2(size)) {
        assert(size < 0x80000000);
        size = roundup_pow_of_two(size);
    }

    buffer = mem_alloc(size);
    if (buffer == NULL)
        return NULL;

    fifo = mem_alloc(sizeof(fifo_buf_t));
    if (fifo == NULL) {
        mem_free(buffer);
        return NULL;
    }
    
    fifo_buf_init(fifo, buffer, size);
    if (fifo == NULL) {
        mem_free(buffer);
        mem_free(fifo);
    }
    
    return fifo;
}

void fifo_buf_free(fifo_buf_t *fifo)
{
    mem_free(fifo->buffer);
    mem_free(fifo);
}

unsigned int __fifo_buf_put(fifo_buf_t *fifo, 
        const unsigned char *buffer, unsigned int len)
{
    unsigned int l;
    /* buffer中空的长度 */
    len = MIN(len, fifo->size - fifo->in + fifo->out);
    /*
    * Ensure that we sample the fifo->out index -before- we
    * start putting bytes into the kfifo.
    */
    mb();
    /* first put the data starting from fifo->in to buffer end */
    l = MIN(len, fifo->size - (fifo->in & (fifo->size - 1)));
    memcpy(fifo->buffer + (fifo->in & (fifo->size - 1)), buffer, l);
    /* then put the rest (if any) at the beginning of the buffer */
    memcpy(fifo->buffer, buffer + l, len - l);

    /*
    * Ensure that we add the bytes to the kfifo -before-
    * we update the fifo->in index.
    */
    wmb();
    fifo->in += len;    //每次累加，到达最大值后溢出，自动转为0
    return len;
}

unsigned int __fifo_buf_get(fifo_buf_t *fifo, 
        const unsigned char *buffer, unsigned int len)
{
    unsigned int l;
    /* 有数据的缓冲区的长度 */
    len = MIN(len, fifo->in - fifo->out);
    /*
    * Ensure that we sample the fifo->in index -before- we
    * start removing bytes from the kfifo.
    */
    rmb();
    /* first get the data from fifo->out until the end of the buffer */
    l = MIN(len, fifo->size - (fifo->out & (fifo->size - 1)));
    memcpy(buffer, fifo->buffer + (fifo->out & (fifo->size - 1)), l);
    /* then get the rest (if any) from the beginning of the buffer */
    memcpy(buffer + l, fifo->buffer, len - l);

    /*
    * Ensure that we remove the bytes from the kfifo -before-
    * we update the fifo->out index.
    */
    mb();
    fifo->out += len;    //每次累加，到达最大值后溢出，自动转为0
    return len;
}