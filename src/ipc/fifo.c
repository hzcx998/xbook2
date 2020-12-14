#include <xbook/fifo.h>
#include <xbook/debug.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <errno.h>
#include <xbook/semaphore.h>
#include <xbook/schedule.h>
#include <sys/ipc.h>

fifo_t *fifo_table;
DEFINE_SEMAPHORE(fifo_mutex, 1);

static fifo_t *fifo_find_by_name(char *name)
{
    fifo_t *fifo;
    int i;
    for (i = 0; i < FIFO_NR; i++) {
        fifo = &fifo_table[i];
        if (fifo->name[0] != '\0') {
            if (!strcmp(fifo->name, name)) {
                return fifo;
            }
        }
    }
    return NULL;
}

static fifo_t *fifo_find_by_id(int fifoid)
{
    fifo_t *fifo;
    int i;
    for (i = 0; i < FIFO_NR; i++) {
        fifo = &fifo_table[i];
        if (fifo->id == fifoid && fifo->name[0] != '\0') { 
            return fifo;
        }
    }
    return NULL;
}

fifo_t *fifo_alloc(char *name)
{
    fifo_t *fifo;
    int i;
    for (i = 0; i < FIFO_NR; i++) {
        fifo = &fifo_table[i];
        if (fifo->name[0] == '\0') {
            fifo->fifo = fifo_buf_alloc(FIFO_SIZE);
            if (fifo->fifo == NULL)
                return NULL;
            memset(fifo->name, 0, FIFO_NAME_LEN);
            strcpy(fifo->name, name);
            atomic_set(&fifo->readref, 0);
            atomic_set(&fifo->writeref, 0);
            return fifo;
        }
    }
    return NULL;
}

int fifo_free(fifo_t *fifo)
{
    if (fifo->fifo) {
        fifo_buf_free(fifo->fifo);
        fifo->fifo = NULL;
    }
    memset(fifo->name, 0, FIFO_NAME_LEN);
    return 0;
}

/**
 * @flags: 获取标志
 *          IPC_CREAT: 如果管道不存在，则创建一个新的管道，否则就打开
 *          IPC_EXCL:  和CREAT一起使用，则要求创建一个新的管道，若已存在，就返回-1。
 *                    相当于在CREAT上面加了一个必须不存在的限定。
 *          IPC_READER: 读者进程
 *          IPC_WRITER: 写者进程
 * 只能有一个读者和一个写者
 * 读者注册时需要检测写者是否在同步等待自己，如果是，就唤醒写者。
 * @return: 成功返回管道id，失败返回-1
 */
int fifo_get(char *name, unsigned long flags)
{
    if (name == NULL)
        return -1;
    char craete_new = 0;
    fifo_t *fifo;
    int retval = -1;
    int rw = -1;
    semaphore_down(&fifo_mutex);
    if (flags & IPC_CREAT) {
        if (flags & IPC_READER) {
            rw = 0;
        } else if (flags & IPC_WRITER) {
            rw = 1;
        } else {
            kprint(PRINT_NOTICE "get fifo %s without reader or writer!\n", name);
        }
        if (flags & IPC_EXCL) {
            craete_new = 1;
        }
        fifo = fifo_find_by_name(name);
        if (fifo) {
            if (craete_new) {
                goto err;
            }
            if (rw == 1) {
                if (fifo->writer == NULL && !atomic_get(&fifo->writeref)) {
                    fifo->writer = task_current;                  
                }
                atomic_inc(&fifo->writeref);
                if (flags & IPC_NOWAIT) {
                    fifo->flags |= (IPC_NOWAIT << 24);
                }
            } else if (rw == 0) {
                if (fifo->reader == NULL && !atomic_get(&fifo->readref)) {
                    fifo->reader = task_current;
                    if (fifo->writer && fifo->writer->state == TASK_BLOCKED && fifo->flags & FIFO_IN_WRITE) {              
                        task_unblock(fifo->writer);
                    }
                }
                atomic_inc(&fifo->readref);
                if (flags & IPC_NOWAIT) {
                    fifo->flags |= (IPC_NOWAIT << 16);
                }
            }
            retval = fifo->id;
        } else {
            fifo = fifo_alloc(name);
            if (fifo == NULL) {
                goto err;
            }
            if (rw == 1) {
                fifo->writer = task_current;
                atomic_set(&fifo->writeref, 1);
                if (flags & IPC_NOWAIT) {
                    fifo->flags |= (IPC_NOWAIT << 24);
                }
            } else if (rw == 0) {
                fifo->reader = task_current;
                atomic_set(&fifo->readref, 1);
                if (flags & IPC_NOWAIT) {
                    fifo->flags |= (IPC_NOWAIT << 16);
                }
            }
            retval = fifo->id;
        }
    }
err:
    semaphore_up(&fifo_mutex);
    return retval;
}

int fifo_put(int fifoid)
{
    fifo_t *fifo;
    semaphore_down(&fifo_mutex);
    fifo = fifo_find_by_id(fifoid);

    if (fifo) {
        mutex_lock(&fifo->mutex);
        if (task_current == fifo->reader) {
            atomic_dec(&fifo->readref);
            if (atomic_get(&fifo->readref) == 0) {
                fifo->reader = NULL;
            }
        } else if (task_current == fifo->writer) {
            atomic_dec(&fifo->writeref);
            if (atomic_get(&fifo->writeref) == 0) {
                fifo->writer = NULL;
            }
        }
        if (fifo->reader == NULL && fifo->writer == NULL) {
            fifo_free(fifo);
        }
        mutex_unlock(&fifo->mutex);
        semaphore_up(&fifo_mutex);        
        return 0;
    }
    semaphore_up(&fifo_mutex);
    return -1;
}

int fifo_write(int fifoid, void *buffer, size_t size)
{
    if (buffer == NULL || !size) {
        kprint(PRINT_ERR "%s: arg error!\n");
        return -1;
    }
    fifo_t *fifo;
    semaphore_down(&fifo_mutex);
    fifo = fifo_find_by_id(fifoid);
    if (fifo == NULL) {
        semaphore_up(&fifo_mutex);
        kprint(PRINT_DEBUG "fifo_write: not found fifo id=%d!\n", fifoid);
        return -1;
    }   
    semaphore_up(&fifo_mutex);
    if (fifo->writer == NULL) {
        kprint(PRINT_ERR "%s: no writer!\n");
        return -1;
    }
    if (fifo->flags & (IPC_NOERROR << 24) && fifo->writer != task_current) {
        kprint(PRINT_ERR "%s: writer no current task!\n");
        return -1;
    }
    fifo->flags |= FIFO_IN_WRITE;
    if (fifo->reader == NULL) {
        if (!(fifo->flags & (IPC_NOSYNC << 24))) {
            task_block(TASK_BLOCKED);
        }
    }

    mutex_lock(&fifo->mutex);
    int left_size = (int )size;
    int off = 0;
    unsigned char *buf = buffer;
    int chunk = 0;
    int wrsize = 0;
    while (left_size > 0) {
        while (fifo_buf_avali(fifo->fifo) <= 0) {
            if (fifo->flags & (IPC_NOWAIT << 24)) {
                fifo->flags &= ~FIFO_IN_WRITE;
                mutex_unlock(&fifo->mutex);
                return -1;
            }
            if (!fifo->reader || atomic_get(&fifo->readref) <= 0) {
                exception_force_self(EXP_CODE_PIPE);
                mutex_unlock(&fifo->mutex);
                return -1; 
            }
            if (fifo->reader->state == TASK_BLOCKED &&
                (fifo->flags & FIFO_IN_READ)) {
                task_unblock(fifo->reader);
            }
            mutex_unlock(&fifo->mutex);
            task_block(TASK_BLOCKED);
            mutex_lock(&fifo->mutex);
        }

        chunk = MIN(left_size, FIFO_SIZE);
        chunk = MIN(chunk, fifo_buf_avali(fifo->fifo));
        chunk = fifo_buf_put(fifo->fifo, buf + off, chunk);
        off += chunk;
        left_size -= chunk;
        wrsize += chunk;
    }
    if (fifo->reader->state == TASK_BLOCKED &&
        (fifo->flags & FIFO_IN_READ)) {
        task_unblock(fifo->reader);
    }
    fifo->flags &= ~FIFO_IN_WRITE;
    mutex_unlock(&fifo->mutex);
    return wrsize;
}

int fifo_read(int fifoid, void *buffer, size_t size)
{
    if (buffer == NULL || !size)
        return -1;
    fifo_t *fifo;
    semaphore_down(&fifo_mutex);
    fifo = fifo_find_by_id(fifoid);
    if (fifo == NULL) {
        semaphore_up(&fifo_mutex);    
        kprint(PRINT_ERR "fifo_read: not found message queue!\n");
        return -1;
    }
    semaphore_up(&fifo_mutex);
    if (fifo->reader == NULL) {
        kprint(PRINT_ERR "fifo_read: reader null!\n");
        return -1;
    }
    if (fifo->flags & (IPC_NOERROR << 16) && (fifo->reader != task_current))
        return -1;
    fifo->flags |= FIFO_IN_READ;
    if (fifo->writer == NULL && (fifo->flags & (IPC_NOSYNC << 16))) {
        kprint(PRINT_DEBUG "fifo_read: don't need sync for reader.\n");  
        return -1;
    }

    mutex_lock(&fifo->mutex);
    int rdsize = 0;
    int chunk;
    while (fifo_buf_len(fifo->fifo) <= 0) {
        if (fifo->flags & (IPC_NOWAIT << 16)) {
            fifo->flags &= ~FIFO_IN_READ;
            mutex_unlock(&fifo->mutex);
            return -1;
        }
        if (!fifo->writer || atomic_get(&fifo->writeref) <= 0) {
            mutex_unlock(&fifo->mutex);
            return -1;
        }
        if (exception_cause_exit(&task_current->exception_manager)) {
            mutex_unlock(&fifo->mutex);
            return -1;
        }
        mutex_unlock(&fifo->mutex);
        task_block(TASK_BLOCKED);
        mutex_lock(&fifo->mutex);
    }

    chunk = MIN(size, FIFO_SIZE);
    chunk = MIN(chunk, fifo_buf_len(fifo->fifo));
    chunk = fifo_buf_get(fifo->fifo, buffer, chunk);
    rdsize += chunk;
    if (fifo->writer && fifo->writer->state == TASK_BLOCKED && 
            (fifo->flags & FIFO_IN_WRITE)) {
        task_unblock(fifo->writer);
    }
    fifo->flags &= ~FIFO_IN_READ;
    mutex_unlock(&fifo->mutex);
    return rdsize;
}

int fifo_set_rdwr(int fifoid, unsigned long arg)
{
    fifo_t *fifo;
    semaphore_down(&fifo_mutex);
    fifo = fifo_find_by_id(fifoid);
    if (fifo) {
        mutex_lock(&fifo->mutex);
        if (arg == IPC_READER) {
            fifo->reader = task_current;
        } else if (arg == IPC_WRITER) {
            fifo->writer = task_current;
        } else {
            mutex_unlock(&fifo->mutex);
            semaphore_up(&fifo_mutex);
            return -1;
        }
        mutex_unlock(&fifo->mutex);
        semaphore_up(&fifo_mutex);        
        return 0;
    }
    semaphore_up(&fifo_mutex);
    return -1;
}

int fifo_set_flags(int fifoid, unsigned int cmd, unsigned long arg)
{
    fifo_t *fifo;
    semaphore_down(&fifo_mutex);
    fifo = fifo_find_by_id(fifoid);
    if (fifo) {
        mutex_lock(&fifo->mutex);
        if (fifo->reader == task_current) {
            fifo->flags |= (arg & 0xff) << 16;
        } else if (fifo->writer == task_current) {
            fifo->flags |= (arg & 0xff) << 24;
        } else {
            mutex_unlock(&fifo->mutex);
            semaphore_up(&fifo_mutex);
            return -1;
        }
        mutex_unlock(&fifo->mutex);
        semaphore_up(&fifo_mutex);        
        return 0;
    }
    semaphore_up(&fifo_mutex);
    return -1;
}

int fifo_ctl(int fifoid, unsigned int cmd, unsigned long arg)
{
    int retval = 0;
    switch (cmd)
    {
    case IPC_SETRW:
        retval = fifo_set_rdwr(fifoid, arg);
        break;
    case IPC_SET:
        retval = fifo_set_flags(fifoid, cmd, arg);
        break;
    default:
        retval = -1;
        break;
    }
    return retval;
}

int fifo_incref(int fifoid)
{
    fifo_t *fifo;
    semaphore_down(&fifo_mutex);
    fifo = fifo_find_by_id(fifoid);
    if (fifo) {
        mutex_lock(&fifo->mutex);
        if (task_current == fifo->reader && atomic_get(&fifo->readref) > 0) {
            atomic_inc(&fifo->readref);
        } else if (task_current == fifo->writer && atomic_get(&fifo->writeref) > 0) {
            atomic_inc(&fifo->writeref);
        } else {  
            dbgprint("[FIFO]: %s: %s: not reader or writer!\n", __func__, task_current->name);
            mutex_unlock(&fifo->mutex);
            semaphore_up(&fifo_mutex);        
            return -1;
        }
        mutex_unlock(&fifo->mutex);
        semaphore_up(&fifo_mutex);        
        return 0;
    }
    dbgprint("[FIFO]: %s: %s: fifo not found!\n", __func__, task_current->name);
    semaphore_up(&fifo_mutex);
    return -1;
}

int fifo_decref(int fifoid)
{
    fifo_t *fifo;
    semaphore_down(&fifo_mutex);
    fifo = fifo_find_by_id(fifoid);
    if (fifo) {
        mutex_lock(&fifo->mutex);
        if (task_current == fifo->reader && atomic_get(&fifo->readref) > 0) {
            atomic_dec(&fifo->readref);
        } else if (task_current == fifo->writer && atomic_get(&fifo->writeref) > 0) {
            atomic_dec(&fifo->writeref);
        } else {  
            dbgprint("[FIFO]: %s: %s: not reader or writer!\n", __func__, task_current->name);
            mutex_unlock(&fifo->mutex);
            semaphore_up(&fifo_mutex);        
            return -1;
        }
        mutex_unlock(&fifo->mutex);
        semaphore_up(&fifo_mutex);        
        return 0;
    }
    dbgprint("[FIFO]: %s: %s: fifo not found!\n", __func__, task_current->name);
    semaphore_up(&fifo_mutex);
    return -1;
}

void fifo_init()
{
    fifo_table = (fifo_t *)mem_alloc(sizeof(fifo_t) * FIFO_NR);
    if (fifo_table == NULL)
        panic(PRINT_EMERG "fifo_init: alloc mem for fifo_table failed! :(\n");
    
    int i;
    for (i = 0; i < FIFO_NR; i++) {
        fifo_table[i].id = 1 + i + i * 2;
        mutexlock_init(&fifo_table[i].mutex);
        memset(fifo_table[i].name, 0, FIFO_NAME_LEN);
        fifo_table[i].fifo = NULL;
        fifo_table[i].reader = NULL;
        fifo_table[i].writer = NULL;
    }
}

static int fifoif_open(void *name, int flags)
{
    char *p = (char *) name;
    unsigned long new_flags = IPC_CREAT;
    if (flags & O_CREAT) {
        new_flags |= IPC_EXCL;
    }
    if (flags & O_RDWR) {
        new_flags |= (IPC_READER | IPC_WRITER);
    } else if (flags & O_RDONLY) {
        new_flags |= IPC_READER;
    } else if (flags & O_WRONLY) {
        new_flags |= IPC_WRITER;
    }
    if (flags & O_NONBLOCK) {
        new_flags |= IPC_NOWAIT;
    }
    int handle = fifo_get(p, new_flags);
    if (handle < 0)
        return -ENODEV;
    return handle;
}

static int fifoif_close(int handle)
{
    return fifo_put(handle);
}

static int fifoif_incref(int handle)
{
    return fifo_incref(handle);
}

static int fifoif_decref(int handle)
{
    return fifo_decref(handle);
}

static int fifoif_read(int handle, void *buf, size_t size)
{
    return fifo_read(handle, buf, size);
}

static int fifoif_write(int handle, void *buf, size_t size)
{
    return fifo_write(handle, buf, size);
}

static int fifoif_ioctl(int handle, int cmd, unsigned long arg)
{
    return fifo_ctl(handle, cmd, arg);
}

static int fifoif_fcntl(int handle, int cmd, long arg)
{
    return fifo_ctl(handle, cmd, arg);
}

fsal_t fifoif = {
    .name       = "fifoif",
    .subtable   = NULL,
    .mkfs       = NULL,
    .mount      = NULL,
    .unmount    = NULL,
    .open       = fifoif_open,
    .close      = fifoif_close,
    .read       = fifoif_read,
    .write      = fifoif_write,
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
    .ioctl      = fifoif_ioctl,
    .fcntl      = fifoif_fcntl,
    .fstat      = NULL,
    .access     = NULL,
    .incref     = fifoif_incref,
    .decref     = fifoif_decref,
};
