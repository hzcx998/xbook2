#include <xbook/fifo.h>
#include <xbook/debug.h>
#include <string.h>
#include <math.h>
#include <xbook/semaphore.h>
#include <sys/ipc.h>

/* debug fifo : 1 enable, 0 disable */
#define DEBUG_LOCAL 0

/* 管道表 */
fifo_t *fifo_table;

/* 保护共享内存的分配与释放 */
DEFINE_SEMAPHORE(fifo_mutex, 1);

/**
 * fifo_find_by_name - 通过名字查找管道  
 * @name: 管道的名字
 * 
 * @return: 如果管道已经在管道表中，就返回管道指针，
 *          没有则返回NULL
 */
static fifo_t *fifo_find_by_name(char *name)
{
    fifo_t *fifo;
    int i;
    for (i = 0; i < FIFO_NR; i++) {
        fifo = &fifo_table[i];
        if (fifo->name[0] != '\0') { /* 有名字才进行比较 */
            if (!strcmp(fifo->name, name)) {
                return fifo;
            }
        }
    }
    return NULL;
}

/**
 * fifo_find_by_id - 通过id查找管道
 * @id: 管道的id
 * 
 * @return: 如果管道已经在管道表中，就返回管道指针，
 *          没有则返回NULL
 */
static fifo_t *fifo_find_by_id(int fifoid)
{
    fifo_t *fifo;
    int i;
    for (i = 0; i < FIFO_NR; i++) {
        fifo = &fifo_table[i];
        /* id相同并且正在使用，才找到 */
        if (fifo->id == fifoid && fifo->name[0] != '\0') { 
            return fifo;
        }
    }
    return NULL;
}

/**
 * fifo_alloc - 分配一个管道
 * @name: 名字
 * 
 * 从管道表中分配一个管道
 * 
 * @return: 成功返回管道结构的地址，失败返回NULL
 */
fifo_t *fifo_alloc(char *name)
{
    fifo_t *fifo;
    int i;
    for (i = 0; i < FIFO_NR; i++) {
        fifo = &fifo_table[i];
        if (fifo->name[0] == '\0') { /* 没有名字才使用 */
            /* 分配fifo缓冲区 */
            fifo->fifo = fifo_buf_alloc(FIFO_SIZE);
            if (fifo->fifo == NULL)
                return NULL;
            /* 设置管道名字 */
            memset(fifo->name, 0, FIFO_NAME_LEN);
            strcpy(fifo->name, name);
            atomic_set(&fifo->readref, 0);
            atomic_set(&fifo->writeref, 0);
            
#if DEBUG_LOCAL == 1
            printk(KERN_DEBUG "fifo_alloc: fifo id=%d\n", fifo->id);
#endif
            return fifo; /* 返回管道 */
        }
    }
    return NULL;
}

/**
 * fifo_free - 释放一个管道
 * @fifo: 管道
 * 
 * @return: 成功返回0，失败返回-1
 */
int fifo_free(fifo_t *fifo)
{
    if (fifo->fifo) {
        fifo_buf_free(fifo->fifo);
        fifo->fifo = NULL;
    }
#if DEBUG_LOCAL == 1    
    printk(KERN_DEBUG "fifo_free: fifo id=%d\n", fifo->id);
#endif
    memset(fifo->name, 0, FIFO_NAME_LEN);

    return 0;
}

/**
 * fifo_get - 获取一个管道
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
int fifo_get(char *name, unsigned long flags)
{
    /* 检测参数 */
    if (name == NULL)
        return -1;
    char craete_new = 0; /* 是否需要创建一个新的管道 */
    fifo_t *fifo;
    int retval = -1;
    int rw = -1;
    /* get fifo table access */
    semaphore_down(&fifo_mutex);
    /* 有创建标志 */
    if (flags & IPC_CREAT) { /* 创建一个新的管道 */
        /* 记录读写者 */
        if (flags & IPC_READER) {
            rw = 0;
        } else if (flags & IPC_WRITER) {
            rw = 1;
        } else { /* 没有读写者，允许通过 */
            //goto err; 
            printk(KERN_NOTICE "get fifo %s without reader or writer!\n", name);
        }

        if (flags & IPC_EXCL) { /* 必须不存在才行 */
            craete_new = 1; /* 需要创建一个新的管道 */
        }
        
        fifo = fifo_find_by_name(name);

        if (fifo) {  /* 管道已经存在 */
            /* 必须创建一个新的，不能是已经存在的，故错误 */
            if (craete_new) {
                goto err;
            }
            /* 根据读写标志设定读写者 */
            if (rw == 1) {
                /* 没有写者就设置写者 */
                if (fifo->writer == NULL && !atomic_get(&fifo->writeref)) {
                    fifo->writer = current_task;                  
                    #if DEBUG_LOCAL == 1                
                    pr_dbg("fifo writer pid %d\n", current_task->pid);
                    #endif
                }
                atomic_inc(&fifo->writeref);
            } else if (rw == 0) {
                /* 没有读者就设置读者 */
                if (fifo->reader == NULL && !atomic_get(&fifo->readref)) {
                    fifo->reader = current_task;
                    
                    #if DEBUG_LOCAL == 1                
                    pr_dbg("fifo reader pid %d\n", current_task->pid);
                    #endif                
                    /* 读者注册时，需要查看是否写者在等待同步中 */
                    if (fifo->writer && fifo->writer->state == TASK_BLOCKED && fifo->flags & FIFO_IN_WRITE) {
                        #if DEBUG_LOCAL == 1
                        printk(KERN_DEBUG "fifo_get: reader register, writer sync wait, wake up writer.\n");
                        #endif                    
                        task_unblock(fifo->writer); /* 唤醒写者 */
                    }
                }
                atomic_inc(&fifo->readref);
            }
            retval = fifo->id; /* 已经存在，那么就返回已经存在的管道的id */
        } else { /* 不存在则创建一个新的 */
            fifo = fifo_alloc(name);
            if (fifo == NULL) {
                goto err;
            }
            /* 新分配的管道，读写者都不存在 */
            if (rw == 1) {
                fifo->writer = current_task;
                atomic_set(&fifo->writeref, 1);
#if DEBUG_LOCAL == 1                
                pr_dbg("fifo writer pid %d\n", current_task->pid);
#endif
            } else if (rw == 0) {
                fifo->reader = current_task;
                atomic_set(&fifo->readref, 1);
#if DEBUG_LOCAL == 1                
                pr_dbg("fifo reader pid %d\n", current_task->pid);
#endif                
            }
            retval = fifo->id; /* 返回管道id */
        }
    }
err:
    semaphore_up(&fifo_mutex);
    //printk("[FIFO]: create fifo %d\n", retval);
    /* 没有创建标志，直接返回错误 */
    return retval;
}

/**
 * fifo_put - 释放一个管道
 * 
 * @fifoid: 管道id
 * 
 * 当读者和写者都已经释放管道后才真正释放管道
 * 
 * @return: 成功返回0，失败返回-1
 */
int fifo_put(int fifoid)
{
    fifo_t *fifo;
    /* get fifo table access */
    semaphore_down(&fifo_mutex);
    fifo = fifo_find_by_id(fifoid);

    if (fifo) {  /* 管道存在 */
        /* 释放期间，不允许读写 */
        mutex_lock(&fifo->mutex);

        /* 管道的读者和写者都无的时候，才真正释放管道 */
        if (current_task == fifo->reader) { /* 是读者 */
            atomic_dec(&fifo->readref);
            if (atomic_get(&fifo->readref) == 0) {
                fifo->reader = NULL;    /* 取消读者身份 */
                #if DEBUG_LOCAL == 1
                printk(KERN_DEBUG "fifo_put: reader closed, free fifo=%d.\n", fifo->id);
                #endif
            }
            
        } else if (current_task == fifo->writer) { /* 是写者 */
            atomic_dec(&fifo->writeref);
            if (atomic_get(&fifo->writeref) == 0) {
                fifo->writer = NULL;    /* 取消写者身份 */
                #if DEBUG_LOCAL == 1
                printk(KERN_DEBUG "fifo_put: writer closed, free fifo=%d.\n", fifo->id);
                #endif
            }
        }
        /* 如果读写者都为空，那么才真正释放管道 */
        if (fifo->reader == NULL && fifo->writer == NULL) {
#if DEBUG_LOCAL == 1
            printk(KERN_DEBUG "fifo_put: both fifo reader and writer closed, free fifo=%d.\n", fifo->id);
#endif
            fifo_free(fifo);
        }
        mutex_unlock(&fifo->mutex);
        
        semaphore_up(&fifo_mutex);        
        return 0;
    }
    
    semaphore_up(&fifo_mutex);
    /* 没有找到管道 */
    return -1;
}

/**
 * fifo_write - 往管道写入数据
 * @fifoid: 管道id
 * @buffer: 缓冲区
 * @size: 数据量
 * @fifoflg: 管道标志：
 *          IPC_NOWAIT: 如果缓冲区已经满了，就直接返回，不阻塞
 *          IPC_NOSYNC: 如果读者没有就绪，就不同步等待读者
 * 
 * 写者需要等待读者就绪后才能写入，不然就需要在write的时候添加IPC_NOSYNC
 * 
 * @return: 成功返回0，失败返回-1
 */
int fifo_write(int fifoid, void *buffer, size_t size, int fifoflg)
{
    /* 参数检查 */
    if (buffer == NULL || !size) {
        printk(KERN_ERR "%s: arg error!\n");
        return -1;
    }
    fifo_t *fifo;
    /* get fifo table access */
    semaphore_down(&fifo_mutex);
    fifo = fifo_find_by_id(fifoid);
    if (fifo == NULL) {  /* not found message */
        semaphore_up(&fifo_mutex);
        printk(KERN_DEBUG "fifo_write: not found fifo id=%d!\n", fifoid);
        return -1;
    }   
    semaphore_up(&fifo_mutex);
    
    /* 写者不能为空 */
    if (fifo->writer == NULL) {
        printk(KERN_ERR "%s: no writer!\n");
        return -1;
    }
        
    if (fifo->flags & (IPC_NOERROR << 24))  /* 没有错误，就是严格的，写者必须是当前进程 */
        if (fifo->writer != current_task) {
            printk(KERN_ERR "%s: writer no current task!\n");
            return -1;
        }

    fifo->flags |= FIFO_IN_WRITE;   /* 管道进入写状态 */
    
    /* 检测读者是否就绪 */
    if (fifo->reader == NULL) {
#if DEBUG_LOCAL == 1
    printk(KERN_DEBUG "fifo_write: reader not ready.\n");
#endif        
        if (!(fifo->flags & (IPC_NOSYNC << 24))) { /* 需要同步等待写者注册 */
#if DEBUG_LOCAL == 1
            printk(KERN_DEBUG "fifo_write: need sync for reader.\n");
#endif
            task_block(TASK_BLOCKED);
        }
    }
#if DEBUG_LOCAL == 1
    printk(KERN_DEBUG "fifo_write: id=%d buffer=%x size=%d flags=%x\n",
        fifoid, buffer, size, fifo->flags);
#endif
    /* 对管道进行操作 */
    mutex_lock(&fifo->mutex); /* 获取管道，只有一个进程可以进入管道 */
    unsigned long flags;

    int left_size = (int )size;
    int off = 0;
    unsigned char *buf = buffer;
    int chunk = 0;
    int wrsize = 0;
    /* 只要还有数据，就不停地写入，直到写完为止 */
    while (left_size > 0) {
        /* 对已有数据的检测，必须是在关闭中断下检测，不能被打断 */
        save_intr(flags);
        chunk = MIN(left_size, FIFO_SIZE); /* 获取一次要写入的数据量 */
        chunk = MIN(chunk, fifo_buf_avali(fifo->fifo)); /* 获取能写入的数据量 */
        
#if DEBUG_LOCAL == 1
        printk(KERN_DEBUG "fifo_write: will write %d bytes.\n", chunk);
#endif
        
        /* 把数据存入缓冲区 */
        chunk = fifo_buf_put(fifo->fifo, buf + off, chunk);
        
        off += chunk;
        left_size -= chunk;
        wrsize += chunk;
#if DEBUG_LOCAL == 1
        printk(KERN_DEBUG "fifo_write: actually write %d bytes.\n", chunk);
#endif
        /* 写入数据后，尝试唤醒阻塞的读者: 有读者，读者阻塞，管道在读 */
        if (fifo->reader && fifo->reader->state == TASK_BLOCKED &&
            (fifo->flags & FIFO_IN_READ))
        {
#if DEBUG_LOCAL == 1
        printk(KERN_DEBUG "fifo_write: wakeup reader pid=%d.\n", fifo->reader->pid);
#endif            
            
            task_unblock(fifo->reader);
        }
        restore_intr(flags); /* 完成对已有数据检测 */

        //save_intr(flags);
        /* 如果fifo缓冲区为已经满了，并且还需要写入数据，就进入抉择阶段 */
        if (fifo_buf_avali(fifo->fifo) <= 0 && left_size > 0) {
            if (fifo->flags & (IPC_NOWAIT << 24)) { /* 如果是不需要等待，就直接返回 */
                fifo->flags &= ~FIFO_IN_WRITE;   /* 管道离开写状态 */
                mutex_unlock(&fifo->mutex); /* 释放管道 */
#if DEBUG_LOCAL == 1
                printk(KERN_DEBUG "fifo_write: write with no wait, return.\n");
#endif               
                return -1;
            }
#if DEBUG_LOCAL == 1
            printk(KERN_DEBUG "fifo_write: buffer full, pid=%d blocked.\n", current_task->pid);
#endif
            /* 阻塞自己，等待有空闲空间后被唤醒 */
            mutex_unlock(&fifo->mutex); /* 释放管道 */
            task_block(TASK_BLOCKED);   /* 阻塞自己 */
            mutex_lock(&fifo->mutex); /* 获取管道 */
        }
    }   
    
    fifo->flags &= ~FIFO_IN_WRITE;   /* 管道离开写状态 */
    
    mutex_unlock(&fifo->mutex); /* 释放管道 */
#if DEBUG_LOCAL == 1
    printk("[FIFO]: %s: size=%d\n", __func__, wrsize);
#endif
    return wrsize;
}

/**
 * fifo_read - 从一个管道中读取数据
 * @fifoid: 管道id
 * @buffer: 缓冲区
 * @size: 缓冲区大小 
 * @fifoflg: 管道标志：
 *          IPC_NOWAIT：管道中没有数据则直接返回
 *          IPC_NOSYNC: 写者没有就绪就直接返回         
 * 
 * @return: 成功返回实际读取的数据量，失败返回-1
 */
int fifo_read(int fifoid, void *buffer, size_t size, int fifoflg)
{
    /* 参数检查 */
    if (buffer == NULL || !size)
        return -1;
    
    fifo_t *fifo;
    /* get fifo table access */
    semaphore_down(&fifo_mutex);
    fifo = fifo_find_by_id(fifoid);
    if (fifo == NULL) {  /* not found message */
        semaphore_up(&fifo_mutex);    
        printk(KERN_ERR "fifo_read: not found message queue!\n");
        return -1;
    }
    semaphore_up(&fifo_mutex);
    
    /* 读者不能为空 */
    if (fifo->reader == NULL) {
        printk(KERN_ERR "fifo_read: reader null!\n");
        return -1;
    }

    if (fifo->flags & (IPC_NOERROR << 16))  /* 没有错误，就是严格的，读者必须是当前进程 */
        if (fifo->reader != current_task) 
            return -1;

    fifo->flags |= FIFO_IN_READ;   /* 管道进入读状态 */
    /* 检测读者是否就绪 */
    if (fifo->writer == NULL) {
#if DEBUG_LOCAL == 1
    printk(KERN_DEBUG "fifo_read: writer not ready.\n");
#endif        
        if (fifo->flags & (IPC_NOSYNC << 16)) { /* 需要同步等待写者注册 */
            printk(KERN_DEBUG "fifo_read: don't need sync for reader.\n");  
            return -1;
        }
    }
#if DEBUG_LOCAL == 1
    printk(KERN_DEBUG "fifo_read: id=%d buffer=%x size=%d flags=%x\n",
        fifoid, buffer, size, fifo->flags);
#endif
    /* 对管道进行操作 */
    mutex_lock(&fifo->mutex); /* 获取管道，只有一个进程可以进入管道 */
    unsigned long flags;
    
    save_intr(flags); /* 对已有数据的检测需要关闭中断 */
    /* 如果缓冲区里面没有数据，就进入抉择截断 */
    if (fifo_buf_len(fifo->fifo) <= 0) {
        if (fifo->flags & (IPC_NOWAIT << 16)) { /* 如果是不需要等待，就直接返回 */
            fifo->flags &= ~FIFO_IN_READ;   /* 管道离开读状态 */
            restore_intr(flags);
            mutex_unlock(&fifo->mutex); /* 释放管道 */
#if DEBUG_LOCAL == 1
            printk(KERN_DEBUG "fifo_read: read with no wait, return.\n");
#endif
            return -1;
        }
#if DEBUG_LOCAL == 1
        printk(KERN_DEBUG "fifo_read: buffer empty, pid=%d blocked.\n", current_task->pid);
#endif
        restore_intr(flags);
        /* 阻塞自己，等待有空闲空间后被唤醒 */
        mutex_unlock(&fifo->mutex); /* 释放管道 */
        task_block(TASK_BLOCKED);   /* 阻塞自己 */
        mutex_lock(&fifo->mutex); /* 获取管道 */
        save_intr(flags);
    }
    restore_intr(flags); /* 完成对已有数据检测 */
    /* 被唤醒后，肯定有数据了 */
    int rdsize = 0;
    int chunk = MIN(size, FIFO_SIZE); /* 获取一次能读取的数据量 */
    chunk = MIN(chunk, fifo_buf_len(fifo->fifo)); /* 获取能读取的数据量 */
#if DEBUG_LOCAL == 1
        printk(KERN_DEBUG "fifo_read: will read %d bytes.\n", chunk);
#endif
    chunk = fifo_buf_get(fifo->fifo, buffer, chunk);
    rdsize += chunk;
#if DEBUG_LOCAL == 1
        printk(KERN_DEBUG "fifo_read: actually read %d bytes.\n", chunk);
#endif   
    /* 读取数据后，尝试唤醒休眠的写者: 有写者，写者阻塞，管道在写 */
    if (fifo->writer && fifo->writer->state == TASK_BLOCKED && 
        (fifo->flags & FIFO_IN_WRITE)) 
    {
#if DEBUG_LOCAL == 1
        printk(KERN_DEBUG "fifo_read: wakeup writer pid=%d.\n", fifo->writer->pid);
#endif
        //restore_intr(flags);
        task_unblock(fifo->writer);
    }
    
    fifo->flags &= ~FIFO_IN_READ;   /* 管道离开读状态 */
    
    mutex_unlock(&fifo->mutex); /* 释放管道 */
#if DEBUG_LOCAL == 1
    printk("[FIFO]: %s: size=%d\n", __func__, rdsize);
#endif
    return rdsize;
}

/**
 * fifo_set_rdwr - 设置管道读写者
 * @fifoid: 管道id
 * @arg: 参数
 */
int fifo_set_rdwr(int fifoid, unsigned long arg)
{
    fifo_t *fifo;
    /* get fifo table access */
    semaphore_down(&fifo_mutex);
    fifo = fifo_find_by_id(fifoid);
    if (fifo) {  /* 管道存在 */
        /* 设置期间，不允许读写 */
        mutex_lock(&fifo->mutex);
        /* 对管道进行设置 */
        if (arg == IPC_READER) {
            fifo->reader = current_task;    /* 设置读者身份 */
        } else if (arg == IPC_WRITER) {
            fifo->writer = current_task;    /* 设置写者身份 */
        } else {
            printk(KERN_NOTICE "%s: arg error!\n", __func__);
            mutex_unlock(&fifo->mutex);
            semaphore_up(&fifo_mutex);
            /* 参数出错 */
            return -1;
        }
        mutex_unlock(&fifo->mutex);
        semaphore_up(&fifo_mutex);        
        return 0;
    }
    semaphore_up(&fifo_mutex);
    /* 没有找到管道 */
    return -1;
}

int fifo_set_flags(int fifoid, unsigned int cmd, unsigned long arg)
{
    fifo_t *fifo;
    /* get fifo table access */
    semaphore_down(&fifo_mutex);
    fifo = fifo_find_by_id(fifoid);
    if (fifo) {  /* 管道存在 */
        /* 设置期间，不允许读写 */
        mutex_lock(&fifo->mutex);
        /* 对管道进行设置 */
        if (fifo->reader == current_task) { /* 是读端设置 */
            fifo->flags |= (arg & 0xff) << 16;
        } else if (fifo->writer == current_task) { /* 是写端设置 */
            fifo->flags |= (arg & 0xff) << 24;
        } else {
            printk(KERN_NOTICE "%s: arg error!\n", __func__);
            mutex_unlock(&fifo->mutex);
            semaphore_up(&fifo_mutex);
            /* 参数出错 */
            return -1;
        }
        
        mutex_unlock(&fifo->mutex);
        semaphore_up(&fifo_mutex);        
        return 0;
    }
    semaphore_up(&fifo_mutex);
    /* 没有找到管道 */
    return -1;
}

/**
 * fifo_ctl - 管道控制
 * @fifoid: 管道id
 * @cmd: 命令
 * @arg: 参数
 * 
 * @return: 成功返回0，失败返回-1
 */
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


/**
 * fifo_grow - 增加管道引用计数
 * @fifoid: 管道id
 * 
 * @return: 成功返回0，失败返回-1
 */
int fifo_grow(int fifoid)
{
    fifo_t *fifo;
    /* get fifo table access */
    semaphore_down(&fifo_mutex);
    fifo = fifo_find_by_id(fifoid);

    if (fifo) {  /* 管道存在 */
        /* 增长期间，不允许读写 */
        mutex_lock(&fifo->mutex);

        /* 管道的读者和写者都无的时候，才真正释放管道 */
        if (current_task == fifo->reader && atomic_get(&fifo->readref) > 0) { /* 是读者 */
            atomic_inc(&fifo->readref);
        } else if (current_task == fifo->writer && atomic_get(&fifo->writeref) > 0) { /* 是写者 */
            atomic_inc(&fifo->writeref);
        } else {  
            pr_dbg("[FIFO]: %s: %s: not reader or writer!\n", __func__, current_task->name);
            mutex_unlock(&fifo->mutex);
            semaphore_up(&fifo_mutex);        
            return -1;
        }

        mutex_unlock(&fifo->mutex);
        semaphore_up(&fifo_mutex);        
        return 0;
    }
    pr_dbg("[FIFO]: %s: %s: fifo not found!\n", __func__, current_task->name);     
    semaphore_up(&fifo_mutex);
    /* 没有找到管道 */
    return -1;
}

/**
 * init_fifo - 初始化管道
 */
void init_fifo()
{
    fifo_table = (fifo_t *)kmalloc(sizeof(fifo_t) * FIFO_NR);
    if (fifo_table == NULL) /* must be ok! */
        panic(KERN_EMERG "init_fifo: alloc mem for fifo_table failed! :(\n");
    
    int i;
    for (i = 0; i < FIFO_NR; i++) {
        fifo_table[i].id = 1 + i + i * 2;
        mutexlock_init(&fifo_table[i].mutex); /* 初始化互斥锁 */ 
        memset(fifo_table[i].name, 0, FIFO_NAME_LEN);
        fifo_table[i].fifo = NULL;
        fifo_table[i].reader = NULL;
        fifo_table[i].writer = NULL;
    }
#if 0
    int fifoid = fifo_get("test", IPC_CREAT);
    if (fifoid < 0) {
        printk(KERN_DEBUG "get fifo failed!\n");
    }
    printk(KERN_DEBUG "get fifo %d.\n", fifoid);
    fifoid = fifo_get("test", IPC_CREAT | IPC_EXCL);
    if (fifoid < 0) {
        printk(KERN_DEBUG "get fifo failed!\n");
    }
    printk(KERN_DEBUG "get fifo %d.\n", fifoid);
    if (fifo_put(fifoid) < 0) {
        printk(KERN_DEBUG "put fifo %d failed!\n", fifoid);
    }
#endif
}
