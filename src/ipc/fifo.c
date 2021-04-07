#include <xbook/fifo.h>
#include <xbook/debug.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ipc.h>

#include <xbook/semaphore.h>
#include <xbook/schedule.h>
#include <xbook/fsal.h>
#include <xbook/path.h>
#include <xbook/file.h>
#include <xbook/dir.h>

#define FIFOFS_PATH  "/fifofs"

typedef struct {
    int handle;
} fifofs_file_extention_t;

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
            keprint(PRINT_NOTICE "get fifo %s without reader or writer!\n", name);
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
            errprint("fifo %s not found!\n", name);
            retval = -ENOFILE;
            goto err;
        }
    }
err:
    semaphore_up(&fifo_mutex);
    return retval;
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
int fifo_get2(char *name, unsigned long flags)
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
            keprint(PRINT_NOTICE "get fifo %s without reader or writer!\n", name);
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

/**
 * 生成一个新的管道文件
 * 成功返回0
 * 如果文件已经存在就返回EEXIST，如果没有可用管道资源，则返回ENOMEN
 */
int fifo_make(char *name, mode_t mode)
{
    if (*name == '\0')
        return -EINVAL;
    char craete_new = 0;
    fifo_t *fifo;
    int retval = -1;
    int rw = -1;
    semaphore_down(&fifo_mutex);
    /* 如果文件已经存在则创建失败 */
    fifo = fifo_find_by_name(name);
    if (fifo) {
        retval = -EEXIST;
        goto err;
    }

    fifo = fifo_alloc(name);
    if (fifo == NULL) {
        retval = -ENOMEM;
        goto err;
    }
    retval = 0;
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
        keprint(PRINT_ERR "%s: arg error!\n");
        return -1;
    }
    fifo_t *fifo;
    semaphore_down(&fifo_mutex);
    fifo = fifo_find_by_id(fifoid);
    if (fifo == NULL) {
        semaphore_up(&fifo_mutex);
        keprint(PRINT_DEBUG "fifo_write: not found fifo id=%d!\n", fifoid);
        return -1;
    }   
    semaphore_up(&fifo_mutex);
    if (fifo->writer == NULL) {
        keprint(PRINT_ERR "%s: no writer!\n");
        return -1;
    }
    if (fifo->flags & (IPC_NOERROR << 24) && fifo->writer != task_current) {
        keprint(PRINT_ERR "%s: writer no current task!\n");
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
        keprint(PRINT_ERR "fifo_read: not found message queue!\n");
        return -1;
    }
    semaphore_up(&fifo_mutex);
    if (fifo->reader == NULL) {
        keprint(PRINT_ERR "fifo_read: reader null!\n");
        return -1;
    }
    if (fifo->flags & (IPC_NOERROR << 16) && (fifo->reader != task_current))
        return -1;
    fifo->flags |= FIFO_IN_READ;
    if (fifo->writer == NULL && (fifo->flags & (IPC_NOSYNC << 16))) {
        keprint(PRINT_DEBUG "fifo_read: don't need sync for reader.\n");  
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

static int fifo_open(void *name, int flags)
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
    return handle;
}

static int fsal_fifofs_mount(char *source, char *target, char *fstype, unsigned long flags);
static int fsal_fifofs_unmount(char *path, unsigned long flags);
static int fifoif_open(void *pathname, int flags);

static int fifoif_incref(int idx)
{
    if (FSAL_BAD_FILE_IDX(idx))
        return -1;
    fsal_file_t *fp = FSAL_IDX2FILE(idx);
    if (FSAL_BAD_FILE(fp)) 
        return -1;
    fifofs_file_extention_t *ext = (fifofs_file_extention_t *) fp->extension;
    return fifo_incref(ext->handle);
}

static int fifoif_decref(int idx)
{
    if (FSAL_BAD_FILE_IDX(idx))
        return -1;
    fsal_file_t *fp = FSAL_IDX2FILE(idx);
    if (FSAL_BAD_FILE(fp)) 
        return -1;
    fifofs_file_extention_t *ext = (fifofs_file_extention_t *) fp->extension;
    return fifo_decref(ext->handle);
}

static int fifoif_read(int idx, void *buf, size_t size)
{
    if (FSAL_BAD_FILE_IDX(idx))
        return -1;
    fsal_file_t *fp = FSAL_IDX2FILE(idx);
    if (FSAL_BAD_FILE(fp)) 
        return -1;
    fifofs_file_extention_t *ext = (fifofs_file_extention_t *) fp->extension;
    return fifo_read(ext->handle, buf, size);
}

static int fifoif_write(int idx, void *buf, size_t size)
{
    if (FSAL_BAD_FILE_IDX(idx))
        return -1;
    fsal_file_t *fp = FSAL_IDX2FILE(idx);
    if (FSAL_BAD_FILE(fp)) 
        return -1;
    fifofs_file_extention_t *ext = (fifofs_file_extention_t *) fp->extension;
    return fifo_write(ext->handle, buf, size);
}

static int fifoif_ioctl(int idx, int cmd, unsigned long arg)
{
    if (FSAL_BAD_FILE_IDX(idx))
        return -1;
    fsal_file_t *fp = FSAL_IDX2FILE(idx);
    if (FSAL_BAD_FILE(fp)) 
        return -1;
    fifofs_file_extention_t *ext = (fifofs_file_extention_t *) fp->extension;
    return fifo_ctl(ext->handle, cmd, arg);
}

static int fifoif_fcntl(int handle, int cmd, long arg)
{
    if (FSAL_BAD_FILE_IDX(handle))
        return -1;
    fsal_file_t *fp = FSAL_IDX2FILE(handle);
    if (FSAL_BAD_FILE(fp)) 
        return -1;
    fifofs_file_extention_t *ext = (fifofs_file_extention_t *) fp->extension;
    return fifo_ctl(ext->handle, cmd, arg);
}

static int fifoif_close(int handle)
{
    if (FSAL_BAD_FILE_IDX(handle))
        return -1;
    fsal_file_t *fp = FSAL_IDX2FILE(handle);
    if (FSAL_BAD_FILE(fp))
        return -1;
    fifofs_file_extention_t *ext = (fifofs_file_extention_t *) fp->extension;
    int retval = fifo_put(ext->handle);
    if (retval < 0)
        warnprint("fifofs: close fd %d, handle %d failed!\n", handle, ext->handle);
    if (fp->extension)
        mem_free(fp->extension);
    fp->extension = NULL;
    if (fsal_file_free(fp) < 0)
        return -1;
    return 0;
}

fsal_t fifofs_fsal = {
    .name       = "fifofs",
    .list       = LIST_HEAD_INIT(fifofs_fsal.list),
    .subtable   = NULL,
    .mkfs       = NULL,
    .mount      = fsal_fifofs_mount,
    .unmount    = fsal_fifofs_unmount,
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
    .fastio     = NULL,
};


static int fsal_fifofs_mount(char *source, char *target, char *fstype, unsigned long flags)
{
    if (strcmp(fstype, "fifofs")) {
        errprint("mount fifofs type %s failed!\n", fstype);
        return -1;
    }
    if (kfile_mkdir(FIFO_DIR_PATH, 0) < 0)
        warnprint("fsal create dir %s failed or dir existed!\n", FIFO_DIR_PATH);
    if (fsal_path_insert(FIFOFS_PATH, target, &fifofs_fsal)) {
        dbgprint("%s: %s: insert path %s failed!\n", FS_MODEL_NAME,__func__, target);
        return -1;
    }
    return 0;
}

static int fsal_fifofs_unmount(char *path, unsigned long flags)
{
    if (fsal_path_remove((void *) path)) {
        dbgprint("%s: %s: remove path %s failed!\n", FS_MODEL_NAME,__func__, path);
        return -1;
    }
    if (kfile_rmdir(FIFO_DIR_PATH) < 0)
        warnprint("fsal remove dir %s failed or dir existed!\n", FIFO_DIR_PATH);
    return 0;
}

/**
 * 将fifofs路径名字转换成管道名。
 * fifofs路径必须是FIFOFS_PATH/xxx
 * 因此需要返回xxx这个设备名
 */
void *fifofs_path_translate(const char *pathname, const char *check_path)
{
    if (!pathname)
        return NULL;
    if (strncmp(pathname, (const char *) check_path, strlen(check_path)) != 0) {   /* 校验路径，不是设备文件系统就退出 */
        return NULL;
    }
    char *p = (char *) pathname;
    p += strlen(check_path);
    while (*p && *p == '/')
        p++;
    return p;
}

int sys_mkfifo(const char *pathname, mode_t mode)
{
    if (!pathname)
        return -EINVAL;
    char *p = fifofs_path_translate((const char *) pathname, FIFO_DIR_PATH);
    if (!p) {
        errprint("sys_mkfifo: file path %s translate faield!\n", pathname);
        return -EINVAL;
    }
    return fifo_make(p, mode);
}

static int fifoif_open(void *pathname, int flags)
{
    char *p = fifofs_path_translate((const char *) pathname, FIFOFS_PATH);
    if (!p) {
        errprint("fifofs: path %s translate faield!\n", pathname);
        return -EINVAL;
    }
    fsal_file_t *fp = fsal_file_alloc();
    if (fp == NULL) {
        errprint("fifofs: alloc file struct failed!\n");
        return -ENOMEM;
    }
    fp->extension = mem_alloc(sizeof(fifofs_file_extention_t));
    if (!fp->extension) {
        errprint("fifofs: alloc file %s extension for open failed!\n", p);
        fsal_file_free(fp);
        return -ENOMEM;
    }
    fp->fsal = &fifofs_fsal;
    int handle = fifo_open(p, flags);
    if (handle < 0) {
        errprint("fifofs: open fifo %s failed!\n", p);
        mem_free(fp->extension);
        fsal_file_free(fp);
        return -ENOFILE;
    }
    fifofs_file_extention_t *ext = (fifofs_file_extention_t *) fp->extension;
    ext->handle = handle;
    return FSAL_FILE2IDX(fp);
}
