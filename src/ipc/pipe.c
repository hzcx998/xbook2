#include <xbook/pipe.h>
#include <xbook/debug.h>
#include <xbook/string.h>
#include <xbook/math.h>
#include <xbook/semaphore.h>
#include <sys/ipc.h>

/* debug pipe : 1 enable, 0 disable */
#define DEBUG_LOCAL 1

/* 管道表 */
pipe_t *pipe_table;

/* 保护共享内存的分配与释放 */
DEFINE_SEMAPHORE(pipe_mutex, 1);

/**
 * pipe_find_by_name - 通过名字查找管道  
 * @name: 管道的名字
 * 
 * @return: 如果管道已经在管道表中，就返回管道指针，
 *          没有则返回NULL
 */
static pipe_t *pipe_find_by_name(char *name)
{
    pipe_t *pipe;
    int i;
    for (i = 0; i < PIPE_NR; i++) {
        pipe = &pipe_table[i];
        if (pipe->name[0] != '\0') { /* 有名字才进行比较 */
            if (!strcmp(pipe->name, name)) {
                return pipe;
            }
        }
    }
    return NULL;
}

/**
 * pipe_find_by_id - 通过id查找管道
 * @id: 管道的id
 * 
 * @return: 如果管道已经在管道表中，就返回管道指针，
 *          没有则返回NULL
 */
static pipe_t *pipe_find_by_id(int pipeid)
{
    pipe_t *pipe;
    int i;
    for (i = 0; i < PIPE_NR; i++) {
        pipe = &pipe_table[i];
        /* id相同并且正在使用，才找到 */
        if (pipe->id == pipeid && pipe->name[0] != '\0') { 
            return pipe;
        }
    }
    return NULL;
}

/**
 * pipe_alloc - 分配一个管道
 * @name: 名字
 * 
 * 从管道表中分配一个管道
 * 
 * @return: 成功返回管道结构的地址，失败返回NULL
 */
pipe_t *pipe_alloc(char *name)
{
    pipe_t *pipe;
    int i;
    for (i = 0; i < PIPE_NR; i++) {
        pipe = &pipe_table[i];
        if (pipe->name[0] == '\0') { /* 没有名字才使用 */
            /* 分配fifo缓冲区 */
            pipe->fifo = fifo_buf_alloc(PIPE_SIZE);
            if (pipe->fifo == NULL)
                return NULL;
            /* 设置管道名字 */
            memcpy(pipe->name, name, PIPE_NAME_LEN);
            pipe->name[PIPE_NAME_LEN - 1] = '\0';
            
#if DEBUG_LOCAL == 1
            printk(KERN_DEBUG "pipe_alloc: pipe id=%d\n", pipe->id);
#endif
            return pipe; /* 返回管道 */
        }
    }
    return NULL;
}

/**
 * pipe_free - 释放一个管道
 * @pipe: 管道
 * 
 * @return: 成功返回0，失败返回-1
 */
int pipe_free(pipe_t *pipe)
{
    if (pipe->fifo) {
        fifo_buf_free(pipe->fifo);
        pipe->fifo = NULL;
    }
#if DEBUG_LOCAL == 1    
    printk(KERN_DEBUG "pipe_free: pipe id=%d\n", pipe->id);
#endif
    memset(pipe->name, 0, PIPE_NAME_LEN);

    return 0;
}

/**
 * pipe_get - 获取一个管道
 * 
 * @name: 管道名
 * @flags: 获取标志
 *          IPC_CREAT: 如果管道不存在，则创建一个新的管道，否则就打开
 *          IPC_EXCL:  和CREAT一起使用，则要求创建一个新的管道，若已存在，就返回-1。
 *                    相当于在CREAT上面加了一个必须不存在的限定。
 *          IPC_READER: 读者进程
 *          IPC_WRITER: 写者进程
 *          
 * 只能有一个读者和一个写者
 * 读者注册时需要检测写者是否在同步等待自己，如果是，就唤醒写者。
 * 
 * @return: 成功返回管道id，失败返回-1
 */
int pipe_get(char *name, unsigned long flags)
{
    /* 检测参数 */
    if (name == NULL)
        return -1;
    char craete_new = 0; /* 是否需要创建一个新的管道 */
    pipe_t *pipe;
    int retval = -1;
    int rw;
    /* get pipe table access */
    semaphore_down(&pipe_mutex);
    /* 有创建标志 */
    if (flags & IPC_CREAT) { /* 创建一个新的管道 */
        /* 记录读写者 */
        if (flags & IPC_READER) {
            rw = 0;
        } else if (flags & IPC_WRITER) {
            rw = 1;
        } else { /* 没有读写者，错误 */
            goto err; 
        }

        if (flags & IPC_EXCL) { /* 必须不存在才行 */
            craete_new = 1; /* 需要创建一个新的管道 */
        }
        
        pipe = pipe_find_by_name(name);

        if (pipe) {  /* 管道已经存在 */
            /* 必须创建一个新的，不能是已经存在的，故错误 */
            if (craete_new) {
                goto err;
            }
            /* 根据读写标志设定读写者 */
            if (rw) {
                if (pipe->writer) /* 写者已经存在，只能有一个写者 */
                    goto err;
                pipe->writer = current_task;
                
            } else {
                if (pipe->reader) /* 读者已经存在，只能有一个读者 */
                    goto err;
                pipe->reader = current_task;  
                /* 读者注册时，需要查看是否写者在等待同步中 */
                if (pipe->writer && pipe->writer->state == TASK_BLOCKED && pipe->flags & PIPE_IN_WRITE) {
#if DEBUG_LOCAL == 1
                    printk(KERN_DEBUG "pipe_get: reader register, writer sync wait, wake up writer.\n");
#endif                    
                    task_unblock(pipe->writer); /* 唤醒写者 */
                }
            }
            retval = pipe->id; /* 已经存在，那么就返回已经存在的管道的id */
        } else { /* 不存在则创建一个新的 */
            pipe = pipe_alloc(name);
            if (pipe == NULL) {
                goto err;
            }
            /* 新分配的管道，读写者都不存在 */
            if (rw) {
                pipe->writer = current_task;
            } else {
                pipe->reader = current_task;    
            }
            retval = pipe->id; /* 返回管道id */
        }
    }
err:
    semaphore_up(&pipe_mutex);

    /* 没有创建标志，直接返回错误 */
    return retval;
}

/**
 * pipe_put - 释放一个管道
 * 
 * @pipeid: 管道id
 * 
 * 当读者和写者都已经释放管道后才真正释放管道
 * 
 * @return: 成功返回0，失败返回-1
 */
int pipe_put(int pipeid)
{
    pipe_t *pipe;
    /* get pipe table access */
    semaphore_down(&pipe_mutex);
    pipe = pipe_find_by_id(pipeid);

    if (pipe) {  /* 管道存在 */
        /* 释放期间，不允许读写 */
        mutex_lock(&pipe->mutex);

        /* 管道的读者和写者都无的时候，才真正释放管道 */
        if (current_task == pipe->reader) { /* 是读者 */
            pipe->reader = NULL;    /* 取消读者身份 */
#if DEBUG_LOCAL == 1
            printk(KERN_DEBUG "pipe_put: reader closed, free pipe=%d.\n", pipe->id);
#endif
        } else if (current_task == pipe->writer) { /* 是写者 */
            pipe->writer = NULL;    /* 取消写者身份 */
#if DEBUG_LOCAL == 1
            printk(KERN_DEBUG "pipe_put: writer closed, free pipe=%d.\n", pipe->id);
#endif
        }
        /* 如果读写者都为空，那么才真正释放管道 */
        if (pipe->reader == NULL && pipe->writer == NULL) {
#if DEBUG_LOCAL == 1
            printk(KERN_DEBUG "pipe_put: both pipe reader and writer closed, free pipe=%d.\n", pipe->id);
#endif
            pipe_free(pipe);
        }
        mutex_unlock(&pipe->mutex);
        
        semaphore_up(&pipe_mutex);        
        return 0;
    }
    
    semaphore_up(&pipe_mutex);
    /* 没有找到管道 */
    return -1;
}

/**
 * pipe_write - 往管道写入数据
 * @pipeid: 管道id
 * @buffer: 缓冲区
 * @size: 数据量
 * @pipeflg: 管道标志：
 *          IPC_NOWAIT: 如果缓冲区已经满了，就直接返回，不阻塞
 *          IPC_NOSYNC: 如果读者没有就绪，就不同步等待读者
 * 
 * 写者需要等待读者就绪后才能写入，不然就需要在write的时候添加IPC_NOSYNC
 * 
 * @return: 成功返回0，失败返回-1
 */
int pipe_write(int pipeid, void *buffer, size_t size, int pipeflg)
{
    /* 参数检查 */
    if (buffer == NULL || !size)
        return -1;

    pipe_t *pipe;
    /* get pipe table access */
    semaphore_down(&pipe_mutex);
    pipe = pipe_find_by_id(pipeid);
    if (pipe == NULL) {  /* not found message */
        semaphore_up(&pipe_mutex);
        printk(KERN_DEBUG "pipe_write: not found pipe id=%d!\n", pipeid);
        return -1;
    }   
    semaphore_up(&pipe_mutex);
    
    /* 写入的时候需要保证是写者写入 */
    if (pipe->writer != current_task)
        return -1;
    
    pipe->flags |= PIPE_IN_WRITE;   /* 管道进入写状态 */
    
    /* 检测读者是否就绪 */
    if (pipe->reader == NULL) {
#if DEBUG_LOCAL == 1
    printk(KERN_DEBUG "pipe_write: reader not ready.\n");
#endif        
        if (!(pipeflg & IPC_NOSYNC)) { /* 需要同步等待写者注册 */
#if DEBUG_LOCAL == 1
            printk(KERN_DEBUG "pipe_write: need sync for reader.\n");
#endif
            task_block(TASK_BLOCKED);
        }
    }
#if DEBUG_LOCAL == 1
    printk(KERN_DEBUG "pipe_write: id=%d buffer=%x size=%d flags=%x\n",
        pipeid, buffer, size, pipeflg);
#endif
    /* 对管道进行操作 */
    mutex_lock(&pipe->mutex); /* 获取管道，只有一个进程可以进入管道 */
    unsigned long flags;
    
    
    int left_size = (int )size;
    int off = 0;
    unsigned char *buf = buffer;
    int chunk;
    
    /* 只要还有数据，就不停地写入，直到写完为止 */
    while (left_size > 0) {
        /* 对已有数据的检测，必须是在关闭中断下检测，不能被打断 */
        save_intr(flags);
        chunk = MIN(left_size, PIPE_SIZE); /* 获取一次要写入的数据量 */
        chunk = MIN(chunk, fifo_buf_avali(pipe->fifo)); /* 获取能写入的数据量 */
        
#if DEBUG_LOCAL == 1
        printk(KERN_DEBUG "pipe_write: will write %d bytes.\n", chunk);
#endif
        
        /* 把数据存入缓冲区 */
        chunk = fifo_buf_put(pipe->fifo, buf + off, chunk);
        off += chunk;
        left_size -= chunk;
#if DEBUG_LOCAL == 1
        printk(KERN_DEBUG "pipe_write: actually write %d bytes.\n", chunk);
#endif
        /* 写入数据后，尝试唤醒阻塞的读者: 有读者，读者阻塞，管道在读 */
        if (pipe->reader && pipe->reader->state == TASK_BLOCKED &&
            (pipe->flags & PIPE_IN_READ))
        {
#if DEBUG_LOCAL == 1
        printk(KERN_DEBUG "pipe_write: wakeup reader pid=%d.\n", pipe->reader->pid);
#endif            
            
            task_unblock(pipe->reader);
        }
        restore_intr(flags); /* 完成对已有数据检测 */

        //save_intr(flags);
        /* 如果fifo缓冲区为已经满了，并且还需要写入数据，就进入抉择阶段 */
        if (fifo_buf_avali(pipe->fifo) <= 0 && left_size > 0) {
            if (pipeflg & IPC_NOWAIT) { /* 如果是不需要等待，就直接返回 */
                pipe->flags &= ~PIPE_IN_WRITE;   /* 管道离开写状态 */
                mutex_unlock(&pipe->mutex); /* 释放管道 */
#if DEBUG_LOCAL == 1
                printk(KERN_DEBUG "pipe_write: write with no wait, return.\n");
#endif               
                return -1;
            }
#if DEBUG_LOCAL == 1
            printk(KERN_DEBUG "pipe_write: buffer full, pid=%d blocked.\n", current_task->pid);
#endif
            /* 阻塞自己，等待有空闲空间后被唤醒 */
            mutex_unlock(&pipe->mutex); /* 释放管道 */
            task_block(TASK_BLOCKED);   /* 阻塞自己 */
            mutex_lock(&pipe->mutex); /* 获取管道 */
        }
    }   
    
    pipe->flags &= ~PIPE_IN_WRITE;   /* 管道离开写状态 */
    
    mutex_unlock(&pipe->mutex); /* 释放管道 */

    return 0;
}

/**
 * pipe_read - 从一个管道中读取数据
 * @pipeid: 管道id
 * @buffer: 缓冲区
 * @size: 缓冲区大小 
 * @pipeflg: 管道标志：
 *          IPC_NOWAIT：管道中没有数据则直接返回
 *          IPC_NOSYNC: 写者没有就绪就直接返回         
 * 
 * @return: 成功返回实际读取的数据量，失败返回-1
 */
int pipe_read(int pipeid, void *buffer, size_t size, int pipeflg)
{
    /* 参数检查 */
    if (buffer == NULL || !size)
        return -1;

    pipe_t *pipe;
    /* get pipe table access */
    semaphore_down(&pipe_mutex);
    pipe = pipe_find_by_id(pipeid);
    if (pipe == NULL) {  /* not found message */
        semaphore_up(&pipe_mutex);    
        printk(KERN_DEBUG "pipe_read: not found message queue!\n");
        
        return -1;
    }
    semaphore_up(&pipe_mutex);
    
    /* 读取的时候需要保证是读者读取 */
    if (pipe->reader != current_task)
        return -1;
    
    pipe->flags |= PIPE_IN_READ;   /* 管道进入读状态 */
    /* 检测读者是否就绪 */
    if (pipe->writer == NULL) {
#if DEBUG_LOCAL == 1
    printk(KERN_DEBUG "pipe_read: writer not ready.\n");
#endif        
        if (pipeflg & IPC_NOSYNC) { /* 需要同步等待写者注册 */
#if DEBUG_LOCAL == 1
            printk(KERN_DEBUG "pipe_read: don't need sync for reader.\n");
#endif      
            return -1;
        }
    }
#if DEBUG_LOCAL == 1
    printk(KERN_DEBUG "pipe_read: id=%d buffer=%x size=%d flags=%x\n",
        pipeid, buffer, size, pipeflg);
#endif
    /* 对管道进行操作 */
    mutex_lock(&pipe->mutex); /* 获取管道，只有一个进程可以进入管道 */
    unsigned long flags;
    
    save_intr(flags); /* 对已有数据的检测需要关闭中断 */
    /* 如果缓冲区里面没有数据，就进入抉择截断 */
    if (fifo_buf_len(pipe->fifo) <= 0) {
        if (pipeflg & IPC_NOWAIT) { /* 如果是不需要等待，就直接返回 */
            pipe->flags &= ~PIPE_IN_READ;   /* 管道离开读状态 */
            restore_intr(flags);
            mutex_unlock(&pipe->mutex); /* 释放管道 */
#if DEBUG_LOCAL == 1
            printk(KERN_DEBUG "pipe_read: read with no wait, return.\n");
#endif               
            return -1;
        }
#if DEBUG_LOCAL == 1
        printk(KERN_DEBUG "pipe_read: buffer empty, pid=%d blocked.\n", current_task->pid);
#endif
        restore_intr(flags);
        /* 阻塞自己，等待有空闲空间后被唤醒 */
        mutex_unlock(&pipe->mutex); /* 释放管道 */
        task_block(TASK_BLOCKED);   /* 阻塞自己 */
        mutex_lock(&pipe->mutex); /* 获取管道 */
        save_intr(flags);
    }
    restore_intr(flags); /* 完成对已有数据检测 */
    /* 被唤醒后，肯定有数据了 */
    int chunk = MIN(size, PIPE_SIZE); /* 获取一次能读取的数据量 */
    chunk = MIN(chunk, fifo_buf_len(pipe->fifo)); /* 获取能读取的数据量 */
#if DEBUG_LOCAL == 1
        printk(KERN_DEBUG "pipe_read: will read %d bytes.\n", chunk);
#endif
    chunk = fifo_buf_get(pipe->fifo, buffer, chunk);
#if DEBUG_LOCAL == 1
        printk(KERN_DEBUG "pipe_read: actually read %d bytes.\n", chunk);
#endif   
    /* 读取数据后，尝试唤醒休眠的写者: 有写者，写者阻塞，管道在写 */
    if (pipe->writer && pipe->writer->state == TASK_BLOCKED && 
        (pipe->flags & PIPE_IN_WRITE)) 
    {
#if DEBUG_LOCAL == 1
        printk(KERN_DEBUG "pipe_read: wakeup writer pid=%d.\n", pipe->writer->pid);
#endif
        //restore_intr(flags);
        task_unblock(pipe->writer);
    }
    
    pipe->flags &= ~PIPE_IN_READ;   /* 管道离开读状态 */
    
    mutex_unlock(&pipe->mutex); /* 释放管道 */

    return chunk;
}

/**
 * init_pipe - 初始化管道
 */
void init_pipe()
{
    pipe_table = (pipe_t *)kmalloc(sizeof(pipe_t) * PIPE_NR);
    if (pipe_table == NULL) /* must be ok! */
        panic(KERN_EMERG "init_pipe: alloc mem for pipe_table failed! :(\n");
    
    int i;
    for (i = 0; i < PIPE_NR; i++) {
        pipe_table[i].id = 1 + i + i * 2;
        mutexlock_init(&pipe_table[i].mutex); /* 初始化互斥锁 */ 
        memset(pipe_table[i].name, 0, PIPE_NAME_LEN);
        pipe_table[i].fifo = NULL;
        pipe_table[i].reader = NULL;
        pipe_table[i].writer = NULL;
    }
#if 0
    int pipeid = pipe_get("test", IPC_CREAT);
    if (pipeid < 0) {
        printk(KERN_DEBUG "get pipe failed!\n");
    }
    printk(KERN_DEBUG "get pipe %d.\n", pipeid);
    pipeid = pipe_get("test", IPC_CREAT | IPC_EXCL);
    if (pipeid < 0) {
        printk(KERN_DEBUG "get pipe failed!\n");
    }
    printk(KERN_DEBUG "get pipe %d.\n", pipeid);
    if (pipe_put(pipeid) < 0) {
        printk(KERN_DEBUG "put pipe %d failed!\n", pipeid);
    }
#endif
}
