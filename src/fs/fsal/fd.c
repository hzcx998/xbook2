#include <fsal/fsal.h>
#include <fsal/fatfs.h>
#include <fsal/dir.h>
#include <fsal/file.h>
#include <fsal/fd.h>

#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include <stdio.h>

#include <xbook/memalloc.h>
#include <xbook/debug.h>
#include <xbook/fs.h>
#include <xbook/schedule.h>

int fs_fd_init(task_t *task)
{
    task->fileman = mem_alloc(sizeof(file_man_t));
    if (task->fileman == NULL) {
        return -1;
    }
    int i;
    for (i = 0; i < LOCAL_FILE_OPEN_NR; i++) {
        task->fileman->fds[i].handle = -1;
        task->fileman->fds[i].flags = 0;
        task->fileman->fds[i].offset = 0;
    }
    memset(task->fileman->cwd, 0, MAX_PATH);
    strcpy(task->fileman->cwd, "/");
    spinlock_init(&task->fileman->lock);
    return 0;
}

int fs_fd_exit(task_t *task)
{
    if (!task->fileman)
        return -1;
    unsigned long irq_flags;
    spin_lock_irqsave(&task->fileman->lock, irq_flags);
    int i;
    for (i = 0; i < LOCAL_FILE_OPEN_NR; i++)
        fsif_degrow(i);
    spin_unlock_irqrestore(&task->fileman->lock, irq_flags);
    mem_free(task->fileman);
    task->fileman = NULL;
    return 0;
}

int fs_fd_copy(task_t *src, task_t *dest)
{
    if (!src->fileman || !dest->fileman) {
        return -1;
    }
    unsigned long irq_flags;
    spin_lock_irqsave(&dest->fileman->lock, irq_flags);
    memcpy(dest->fileman->cwd, src->fileman->cwd, MAX_PATH);
    int i;
    for (i = 0; i < LOCAL_FILE_OPEN_NR; i++) {
        if (src->fileman->fds[i].flags != 0) {
            dest->fileman->fds[i].handle = src->fileman->fds[i].handle;
            dest->fileman->fds[i].flags = src->fileman->fds[i].flags;
            dest->fileman->fds[i].offset = src->fileman->fds[i].offset;
            fsif_grow(i);
        }
    }
    spin_unlock_irqrestore(&dest->fileman->lock, irq_flags);
    return 0;
}

/**
 * fs_fd_reinit - 重新初始化只保留前3个fd
 */
int fs_fd_reinit(task_t *cur)
{
    if (!cur->fileman) {
        return -1;
    }
    unsigned long irq_flags;
    spin_lock_irqsave(&cur->fileman->lock, irq_flags);
    int i;
    for (i = 0; i < LOCAL_FILE_OPEN_NR; i++) {
        if (cur->fileman->fds[i].flags != 0) {
            if (i >= 3)
                sys_close(i);
        }
    }
    spin_unlock_irqrestore(&cur->fileman->lock, irq_flags);
    return 0;
}

int fsal_fd_alloc()
{
    task_t *cur = task_current;
    unsigned long irq_flags;
    spin_lock_irqsave(&cur->fileman->lock, irq_flags);
    int i;
    for (i = 0; i < LOCAL_FILE_OPEN_NR; i++) {
        if (cur->fileman->fds[i].flags == 0) {
            cur->fileman->fds[i].flags = 1;
            cur->fileman->fds[i].handle = -1;
            cur->fileman->fds[i].offset = 0;
            spin_unlock_irqrestore(&cur->fileman->lock, irq_flags);
            return i;
        }
    }
    spin_unlock_irqrestore(&cur->fileman->lock, irq_flags);
    return -1;
}

int fsal_fd_free(int fd)
{
    task_t *cur = task_current;
    if (OUT_RANGE(fd, 0, LOCAL_FILE_OPEN_NR))
        return -1;
    unsigned long irq_flags;
    spin_lock_irqsave(&cur->fileman->lock, irq_flags);
    if (cur->fileman->fds[fd].flags == 0) {
        spin_unlock_irqrestore(&cur->fileman->lock, irq_flags);
        return -1;
    }
    cur->fileman->fds[fd].handle = -1;
    cur->fileman->fds[fd].flags = 0;
    cur->fileman->fds[fd].offset = 0;
    spin_unlock_irqrestore(&cur->fileman->lock, irq_flags);
    return 0;
}

/**
 * local_fd_install - 安装到进程本地文件描述符表
 */
int local_fd_install(int resid, unsigned int flags)
{
    if (OUT_RANGE(resid, 0, FSAL_FILE_OPEN_NR))
        return -1;
    int fd = fsal_fd_alloc();
    if (fd < 0)
        return -1;
    task_t *cur = task_current;
    unsigned long irq_flags;
    spin_lock_irqsave(&cur->fileman->lock, irq_flags);
    cur->fileman->fds[fd].handle = resid;
    cur->fileman->fds[fd].flags |= flags;
    /* 根据不同的标志设置不同的fsal指针 */
    spin_unlock_irqrestore(&cur->fileman->lock, irq_flags);
    return fd;
}

/**
 * local_fd_install_to - 安装到进程本地文件描述符表，并指明要安装到的fd
 */
int local_fd_install_to(int resid, int newfd, unsigned int flags)
{
    if (OUT_RANGE(resid, 0, FSAL_FILE_OPEN_NR))
        return -1;
    if (OUT_RANGE(newfd, 0, LOCAL_FILE_OPEN_NR))
        return -1;
    task_t *cur = task_current;
    unsigned long irq_flags;
    spin_lock_irqsave(&cur->fileman->lock, irq_flags);
    cur->fileman->fds[newfd].handle = resid;
    cur->fileman->fds[newfd].flags = FILE_FD_ALLOC;
    cur->fileman->fds[newfd].flags |= flags;
    cur->fileman->fds[newfd].offset = 0;
    spin_unlock_irqrestore(&cur->fileman->lock, irq_flags);
    return newfd;
}

int local_fd_uninstall(int local_fd)
{
    if (OUT_RANGE(local_fd, 0, LOCAL_FILE_OPEN_NR))
        return -1;
    return fsal_fd_free(local_fd);
}

file_fd_t *fd_local_to_file(int local_fd)
{
    if (OUT_RANGE(local_fd, 0, LOCAL_FILE_OPEN_NR))
        return NULL;

    task_t *cur = task_current;
    return &cur->fileman->fds[local_fd];
}

int handle_to_local_fd(int handle, unsigned int flags)
{
    task_t *cur = task_current;
    unsigned long irq_flags;
    spin_lock_irqsave(&cur->fileman->lock, irq_flags);
    file_fd_t *fdptr;
    int i;
    for (i = 0; i < LOCAL_FILE_OPEN_NR; i++) {
        fdptr = &cur->fileman->fds[i];
        if ((fdptr->handle == handle) && (fdptr->flags & flags)) {
            spin_unlock_irqrestore(&cur->fileman->lock, irq_flags);
            return i;   /* find the local fd */
        }
    }
    spin_unlock_irqrestore(&cur->fileman->lock, irq_flags);
    return -1;
}
