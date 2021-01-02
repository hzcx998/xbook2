#include <xbook/pipe.h>
#include <xbook/debug.h>
#include <xbook/schedule.h>

#include <xbook/fsal.h>
#include <types.h>
#include <fcntl.h>
#include <unistd.h>

LIST_HEAD(pipe_list_head);

static kobjid_t pipe_next_id = 0;
 
pipe_t *create_pipe()
{
    pipe_t *pipe = mem_alloc(sizeof(pipe_t));
    if (pipe == NULL) {
        return NULL;
    }
    pipe->fifo = fifo_buf_alloc(PIPE_SIZE);
    if (pipe->fifo == NULL) {
        mem_free(pipe);
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
    list_add_tail(&pipe->list, &pipe_list_head);
    return pipe;
}

int destroy_pipe(pipe_t *pipe)
{
    if (!pipe)
        return -1;
    list_del_init(&pipe->list);
    fifo_buf_free(pipe->fifo);
    mem_free(pipe);
    return 0;
}

int pipe_clear(pipe_t *pipe)
{
    if (!pipe)
        return -1;
    atomic_set(&pipe->read_count, 1);
    atomic_set(&pipe->write_count, 1);
    pipe->rdflags = 0;
    pipe->wrflags = 0;
    pipe->flags = 0;
    mutexlock_init(&pipe->mutex);
    wait_queue_init(&pipe->wait_queue);
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
int pipe_read(kobjid_t pipeid, void *buffer, size_t bytes)
{
    if (!buffer || !bytes)
        return -1;
    pipe_t *pipe = pipe_find(pipeid);
    if (pipe == NULL) {
        keprint(PRINT_ERR "%s: pipe %d not found!\n", __func__, pipeid);
        return -1;
    }
    if (atomic_get(&pipe->read_count) <= 0)  {
        keprint(PRINT_ERR "%s: pipe %d reader is zero!\n", __func__, pipeid);
        return -1;
    }

    int rdsize = 0;
    int chunk;
    mutex_lock(&pipe->mutex);
    
    while (fifo_buf_len(pipe->fifo) <= 0) {
        if (atomic_get(&pipe->write_count) <= 0) {
            mutex_unlock(&pipe->mutex);
            return 0;
        }
        if (pipe->rdflags & PIPE_NOWAIT) {
            mutex_unlock(&pipe->mutex);
            return -1;
        }
        if (exception_cause_exit(&task_current->exception_manager)) {
            mutex_unlock(&pipe->mutex);
            return -1;
        }
        wait_queue_add(&pipe->wait_queue, task_current);
        mutex_unlock(&pipe->mutex);
        task_block(TASK_BLOCKED);
        mutex_lock(&pipe->mutex);
    }
    chunk = min(bytes, PIPE_SIZE);
    chunk = min(chunk, fifo_buf_len(pipe->fifo));
    chunk = fifo_buf_get(pipe->fifo, buffer, chunk);
    rdsize += chunk;
    
    if (atomic_get(&pipe->write_count) > 0) {
        if (wait_queue_length(&pipe->wait_queue) > 0)
            wait_queue_wakeup(&pipe->wait_queue);
    }
    mutex_unlock(&pipe->mutex);
    return rdsize;
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
    pipe_t *pipe = pipe_find(pipeid);
    if (pipe == NULL) {
        keprint(PRINT_ERR "%s: pipe %d not found!\n", __func__, pipeid);
        return -1;
    }
    if (atomic_get(&pipe->write_count) <= 0) {
        keprint(PRINT_ERR "%s: pipe %d writer is zero!\n", __func__, pipeid);
        return -1;
    }
        
    if (atomic_get(&pipe->read_count) <= 0) {
        exception_force_self(EXP_CODE_PIPE);
        return -1;
    } 
        
    mutex_lock(&pipe->mutex);

    int left_size = (int )bytes;
    int off = 0;
    unsigned char *buf = buffer;
    int chunk = 0;
    int wrsize = 0;
    while (left_size > 0) {
        while (fifo_buf_avali(pipe->fifo) <= 0) {
            if ((pipe->wrflags & PIPE_NOWAIT) || 
                exception_cause_exit(&task_current->exception_manager)) {
                mutex_unlock(&pipe->mutex);
                return -1;
            }
            if (atomic_get(&pipe->read_count) <= 0) {
                exception_force_self(EXP_CODE_PIPE);
                mutex_unlock(&pipe->mutex);
                return -1;
            }
            if (atomic_get(&pipe->read_count) > 0) {
                if (wait_queue_length(&pipe->wait_queue) > 0)
                    wait_queue_wakeup(&pipe->wait_queue);
            }
            wait_queue_add(&pipe->wait_queue, task_current);
            mutex_unlock(&pipe->mutex);
            task_block(TASK_BLOCKED);
            mutex_lock(&pipe->mutex);
        }
        chunk = min(left_size, PIPE_SIZE);
        chunk = min(chunk, fifo_buf_avali(pipe->fifo));
        chunk = fifo_buf_put(pipe->fifo, buf + off, chunk);
        off += chunk;
        left_size -= chunk;
        wrsize += chunk;
    }
    if (atomic_get(&pipe->read_count) > 0) {
        if (wait_queue_length(&pipe->wait_queue) > 0)
            wait_queue_wakeup(&pipe->wait_queue);
    }
    mutex_unlock(&pipe->mutex);
    return wrsize;
}

int pipe_close(kobjid_t pipeid, int rw)
{
    pipe_t *pipe = pipe_find(pipeid);
    if (pipe == NULL) {
        return -1;
    }
    if (rw) {
        atomic_dec(&pipe->write_count);
    } else {
        atomic_dec(&pipe->read_count);
    }
    if (atomic_get(&pipe->write_count) <= 0 && atomic_get(&pipe->read_count) <= 0) {
        destroy_pipe(pipe);
    }
    return 0;
}

int pipe_ioctl(kobjid_t pipeid, unsigned int cmd, unsigned long arg, int rw)
{
    pipe_t *pipe = pipe_find(pipeid);
    if (pipe == NULL) {
        return -1;
    }
    mutex_lock(&pipe->mutex);
    int err = -1;
    switch (cmd) {
    case F_SETFL:
        if ((*(unsigned long *)arg) & O_NONBLOCK) {
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

int pipe_incref(kobjid_t pipeid, int rw)
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

int pipe_decref(kobjid_t pipeid, int rw)
{
    pipe_t *pipe = pipe_find(pipeid);
    if (pipe == NULL) {
        return -1;
    }
    mutex_lock(&pipe->mutex);
    if (rw) 
        atomic_dec(&pipe->write_count);
    else
        atomic_dec(&pipe->read_count);
    mutex_unlock(&pipe->mutex);
    return 0;
}

/* 接口封装 */
static int pipeif_rd_close(int handle)
{
    return pipe_close(handle, 0);
}

static int pipeif_wr_close(int handle)
{
    return pipe_close(handle, 1);
}

static int pipeif_rd_incref(int handle)
{
    return pipe_incref(handle, 0);
}

static int pipeif_wr_incref(int handle)
{
    return pipe_incref(handle, 1);
}

static int pipeif_rd_decref(int handle)
{
    return pipe_decref(handle, 0);
}

static int pipeif_wr_decref(int handle)
{
    return pipe_decref(handle, 1);
}

static int pipeif_read(int handle, void *buf, size_t size)
{
    return pipe_read(handle, buf, size);
}

static int pipeif_write(int handle, void *buf, size_t size)
{
    return pipe_write(handle, buf, size);
}

static int pipeif_rd_ioctl(int handle, int cmd, unsigned long arg)
{
    return pipe_ioctl(handle, cmd, arg, 0);
}

static int pipeif_wr_ioctl(int handle, int cmd, unsigned long arg)
{
    return pipe_ioctl(handle, cmd, arg, 1);
}

static int pipeif_rd_fcntl(int handle, int cmd, long arg)
{
    return pipe_ioctl(handle, cmd, (unsigned long) arg, 0);
}

static int pipeif_wr_fcntl(int handle, int cmd, long arg)
{
    return pipe_ioctl(handle, cmd, (unsigned long) arg, 1);
}

fsal_t pipeif_rd = {
    .name       = "pipeif_rd",
    .subtable   = NULL,
    .mkfs       = NULL,
    .mount      = NULL,
    .unmount    = NULL,
    .open       = NULL,
    .close      = pipeif_rd_close,
    .read       = pipeif_read,
    .write      = NULL,
    .lseek      = NULL,
    .opendir    = NULL,
    .closedir   = NULL,
    .readdir    = NULL,
    .mkdir      = NULL,
    .unlink     = NULL,
    .rename     = NULL,
    .ftruncate  = NULL,
    .fsync      = NULL,
    .state      = NULL,
    .chmod      = NULL,
    .fchmod     = NULL,
    .utime      = NULL,
    .feof       = NULL,
    .ferror     = NULL,
    .ftell      = NULL,
    .fsize      = NULL,
    .rewind     = NULL,
    .rewinddir  = NULL,
    .rmdir      = NULL,
    .chdir      = NULL,
    .ioctl      = pipeif_rd_ioctl,
    .fcntl      = pipeif_rd_fcntl,
    .fstat      = NULL,
    .access     = NULL,
    .incref     = pipeif_rd_incref,
    .decref     = pipeif_rd_decref,
};

fsal_t pipeif_wr = {
    .name       = "pipeif_wr",
    .subtable   = NULL,
    .mkfs       = NULL,
    .mount      = NULL,
    .unmount    = NULL,
    .open       = NULL,
    .close      = pipeif_wr_close,
    .read       = NULL,
    .write      = pipeif_write,
    .lseek      = NULL,
    .opendir    = NULL,
    .closedir   = NULL,
    .readdir    = NULL,
    .mkdir      = NULL,
    .unlink     = NULL,
    .rename     = NULL,
    .ftruncate  = NULL,
    .fsync      = NULL,
    .state      = NULL,
    .chmod      = NULL,
    .fchmod     = NULL,
    .utime      = NULL,
    .feof       = NULL,
    .ferror     = NULL,
    .ftell      = NULL,
    .fsize      = NULL,
    .rewind     = NULL,
    .rewinddir  = NULL,
    .rmdir      = NULL,
    .chdir      = NULL,
    .ioctl      = pipeif_wr_ioctl,
    .fcntl      = pipeif_wr_fcntl,
    .fstat      = NULL,
    .access     = NULL,
    .incref     = pipeif_wr_incref,
    .decref     = pipeif_wr_decref,
};
