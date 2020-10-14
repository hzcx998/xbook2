#include <xbook/pipe.h>
#include <xbook/trigger.h>
#include <xbook/debug.h>
#include <xbook/schedule.h>

#include <fsal/fsal.h>
#include <types.h>
#include <fcntl.h>
#include <unistd.h>

LIST_HEAD(pipe_list_head);

static kobjid_t pipe_next_id = 0;
 
pipe_t *create_pipe()
{
    pipe_t *pipe = kmalloc(sizeof(pipe_t));
    if (pipe == NULL) {
        return NULL;
    }
    pipe->fifo = fifo_buf_alloc(PIPE_SIZE);
    if (pipe->fifo == NULL) {
        kfree(pipe);
        return NULL;
    }
    atomic_set(&pipe->read_count, 1);
    atomic_set(&pipe->write_count, 1);
    pipe->rdflags = 0;
    pipe->wrflags = 0;
    pipe->flags = 0;
    pipe->id = pipe_next_id;
    pipe_next_id++;
    mutexlock_init(&pipe->mutex);
    wait_queue_init(&pipe->wait_queue);
    /* add to pipe list */
    list_add_tail(&pipe->list, &pipe_list_head);
    return pipe;
}

int destroy_pipe(pipe_t *pipe)
{
    if (!pipe)
        return -1;
    list_del_init(&pipe->list);
    fifo_buf_free(pipe->fifo);
    kfree(pipe);
    return 0;
}

pipe_t *pipe_find(kobjid_t id)
{

    pipe_t *pipe;
    list_for_each_owner (pipe, &pipe_list_head, list) {
        if (pipe->id == id)
            return pipe;
    }
    return NULL;
}

/**
 * 从管道读取数据。
 * 1.如果管道读端没有打开，读取返回-1
 * 2.如果管道是阻塞状态，管道有数据则返回读取的数据量，不管写端是否关闭。
 *   如果是无阻塞状态，没有数据就不阻塞，而是返回
 * 3.管道中没有数据，如果写端全部关闭，则返回0.
 * 4.如果写端没有全部关闭，则阻塞。
 */
int __pipe_read(kobjid_t pipeid, void *buffer, size_t bytes)
{
    if (!buffer || !bytes)
        return -1;
    /* find the pipe id */
    pipe_t *pipe = pipe_find(pipeid);
    if (pipe == NULL) {
        printk(KERN_ERR "%s: pipe %d not found!\n", __func__, pipeid);
        return -1;
    }
    if (atomic_get(&pipe->read_count) <= 0)  {
        printk(KERN_ERR "%s: pipe %d reader is zero!\n", __func__, pipeid);
        return -1;
    }

    int rdsize = 0;
    int chunk;
    // 
    
    mutex_lock(&pipe->mutex);

    /* 没有数据就要检查写端状态，如果关闭则返回0，不然就读取阻塞，等待有数据后，读取数据返回 */
    if (fifo_buf_len(pipe->fifo) <= 0) {
        /* 写端全部被关闭，返回0 */
        if (atomic_get(&pipe->write_count) <= 0) {
            mutex_unlock(&pipe->mutex);
            return 0;
        }
        if (pipe->rdflags & PIPE_NOWAIT) {  /* 无阻塞 */
            mutex_unlock(&pipe->mutex);
            return -1;
        }
        printk(KERN_ERR "%s: pipe %d no data!\n", __func__, pipeid);
        
        /* 添加到等待队列 */
        wait_queue_add(&pipe->wait_queue, current_task);
        mutex_unlock(&pipe->mutex);
        task_block(TASK_BLOCKED); /* 阻塞自己，等待唤醒 */
        mutex_lock(&pipe->mutex);
    }
    /* 获取缓冲区大小，如果有数据，则直接读取数据，然后返回 */
    
    chunk = min(bytes, PIPE_SIZE); /* 获取一次能读取的数据量 */
    chunk = min(chunk, fifo_buf_len(pipe->fifo)); /* 获取能读取的数据量 */
    chunk = fifo_buf_get(pipe->fifo, buffer, chunk);
    rdsize += chunk;
    /* 读取完数据后，此时写者可能因为管道满了而阻塞，所以尝试唤醒 */
    if (atomic_get(&pipe->write_count) > 0) {
        if (wait_queue_length(&pipe->wait_queue) > 0)
            wait_queue_wakeup(&pipe->wait_queue);
    }
    
    mutex_unlock(&pipe->mutex);

    return rdsize;
}

int pipe_read(kobjid_t pipeid, void *buffer, size_t bytes)
{
    char *buf = (char *) buffer;
    int rd = 0, tmp;
    while (bytes > 0) {
        tmp = __pipe_read(pipeid, buf, bytes);
        if (tmp < 0 && rd == 0) { // 第一次读取就出错
            return -1;
        } else if (tmp < 0) { // 还是没有数据
            return rd;
        }
        rd += tmp;
        buf += tmp;
        bytes -= tmp;
    }
    return rd;
}


/**
 * 从管道写入数据。
 * 1.如果管道写端没有打开，读取返回-1
 * 2.如果读端全关闭，则触发进程异常
 * 3.如果管道是阻塞状态，管道满则阻塞。
 *   如果是无阻塞状态，管道满则不阻塞，直接返回-1
 * 4.如果管道未满，则写入数据，并返回实际的数据量
 */
int pipe_write(kobjid_t pipeid, void *buffer, size_t bytes)
{
    if (!buffer || !bytes)
        return -1;
    /* find the pipe id */
    pipe_t *pipe = pipe_find(pipeid);
    if (pipe == NULL) {
        printk(KERN_ERR "%s: pipe %d not found!\n", __func__, pipeid);
        return -1;
    }
    if (atomic_get(&pipe->write_count) <= 0) {
        printk(KERN_ERR "%s: pipe %d writer is zero!\n", __func__, pipeid);
        return -1;
    }
        
    if (atomic_get(&pipe->read_count) <= 0) {
        /* 没有读端时写入，触发管道异常 */
        sys_trigger_active(TRIGHSOFT, current_task->pid);
        return -1;
    } 
        
    mutex_lock(&pipe->mutex);

    int left_size = (int )bytes;
    int off = 0;
    unsigned char *buf = buffer;
    int chunk = 0;
    int wrsize = 0;
    /* 只要还有数据，就不停地写入，直到写完为止 */
    while (left_size > 0) {
        chunk = min(left_size, PIPE_SIZE); /* 获取一次要写入的数据量 */
        chunk = min(chunk, fifo_buf_avali(pipe->fifo)); /* 获取能写入的数据量 */

        /* 把数据存入缓冲区 */
        chunk = fifo_buf_put(pipe->fifo, buf + off, chunk);
        
        off += chunk;
        left_size -= chunk;
        wrsize += chunk;
        /* try wakeup reader */
        if (atomic_get(&pipe->read_count) > 0) {
            if (wait_queue_length(&pipe->wait_queue) > 0)
                wait_queue_wakeup(&pipe->wait_queue);
        }
        
        /* 如果fifo缓冲区为已经满了，并且还需要写入数据，就进入抉择阶段 */
        if (fifo_buf_avali(pipe->fifo) <= 0 && left_size > 0) {
            if (pipe->wrflags & PIPE_NOWAIT) {  /* 无阻塞 */
                mutex_unlock(&pipe->mutex);
                return -1;
            }
            /* 添加到等待队列 */
            wait_queue_add(&pipe->wait_queue, current_task);
            mutex_unlock(&pipe->mutex);
            task_block(TASK_BLOCKED); /* 阻塞自己，等待唤醒 */
            mutex_lock(&pipe->mutex);
        }
    }
    mutex_unlock(&pipe->mutex);
    return wrsize;
}

/**
 * pipe_close - 关闭管道
 * @pipeid: 管道id
 * @rw: 读写端口（0：读端，1：写端）
 * 
 * 成功关闭返回0，失败返回-1
 */
int pipe_close(kobjid_t pipeid, int rw)
{
    /* find the pipe id */
    pipe_t *pipe = pipe_find(pipeid);
    if (pipe == NULL) {
        return -1;
    }
    if (rw) {
        atomic_dec(&pipe->write_count);
        //pr_dbg("[pipe]: %s: close write pipe %d.\n", __func__, atomic_get(&pipe->write_count));
    } else {
        atomic_dec(&pipe->read_count);
        //pr_dbg("[pipe]: %s: close read pipe %d.\n", __func__, atomic_get(&pipe->read_count));
    }
    /* both closed */
    if (atomic_get(&pipe->write_count) <= 0 && atomic_get(&pipe->read_count) <= 0) {
        destroy_pipe(pipe);
        //pr_dbg("[pipe]: %s: destroy pipe.\n", __func__);
    }
    return 0;
}

/**
 * pipe_close - 关闭管道
 * @pipeid: 管道id
 * @rw: 读写端口（0：读端，1：写端）
 * 
 * 读写端都关闭返回1，只关闭一个返回0，失败返回-1
 */
int pipe_ioctl(kobjid_t pipeid, unsigned int cmd, unsigned long arg, int rw)
{
    /* find the pipe id */
    pipe_t *pipe = pipe_find(pipeid);
    if (pipe == NULL) {
        return -1;
    }
    mutex_lock(&pipe->mutex);
    int err = -1;
    switch (cmd) {
    case F_SETFL:
        if (arg & O_NONBLOCK) {
            if (rw)
                pipe->wrflags |= PIPE_NOWAIT;
            else 
                pipe->rdflags |= PIPE_NOWAIT;
        }
        err = 0;
        break;
    default:
        break;
    }
    mutex_unlock(&pipe->mutex);
    return err;
}

int pipe_grow(kobjid_t pipeid, int rw)
{
    pipe_t *pipe = pipe_find(pipeid);
    if (pipe == NULL) {
        return -1;
    }

    mutex_lock(&pipe->mutex);
    if (rw) 
        atomic_inc(&pipe->write_count);
    else
        atomic_inc(&pipe->read_count);
    mutex_unlock(&pipe->mutex);
    return 0;
}