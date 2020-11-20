#include <fsal/fsal.h>
#include <fsal/fd.h>
#include <xbook/fs.h>
#include <xbook/task.h>
#include <xbook/debug.h>
#include <xbook/driver.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <lwip/sockets.h>
#include <xbook/fifo.h>
#include <xbook/pipe.h>
#include <xbook/safety.h>
#include <sys/ipc.h>
#include <sys/ioctl.h>

// #define DEBUG_FSIF

// #define FSIF_USER_CHECK


int sys_open(const char *path, int flags)
{
    if (!path)
        return -EINVAL; 
    #ifdef FSIF_USER_CHECK
    if (mem_copy_from_user(NULL, (void *)path, MAX_PATH) < 0)
        return -EINVAL;
    #endif
    int handle;
    int fd = -1;
    unsigned long new_flags;
    if (O_FIFO & flags) {
        /* 去掉根目录 */
        char *p = (char *) path;
        if (*p == '/')
            p++;
        
        /* 有创建标志 */
        new_flags = IPC_CREAT;
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
        handle = fifo_get(p, new_flags);
        if (handle < 0)
            return -ENODEV;
        fd = local_fd_install(handle, FILE_FD_FIFO);
    } else {
        handle = fsif.open((void *)path, flags);
        if (handle < 0)
            return -ENOFILE;
        fd = local_fd_install(handle, FILE_FD_NORMAL);
    }
    return fd;
}

int sys_close(int fd)
{
    file_fd_t *ffd = fd_local_to_file(fd);
    if (FILE_FD_IS_BAD(ffd))
        return -EINVAL;
    if (!ffd->fsal->close)
        return -ENOSYS;
    if (ffd->fsal->close(ffd->handle) < 0)
        return -1;
    return local_fd_uninstall(fd);
}

int sys_read(int fd, void *buffer, size_t nbytes)
{
    if (fd < 0 || !nbytes || !buffer)
        return -EINVAL;
    #ifdef FSIF_USER_CHECK
    if (mem_copy_to_user(buffer, NULL, nbytes) < 0)
        return -EINVAL;
    #endif
    file_fd_t *ffd = fd_local_to_file(fd);
    if (FILE_FD_IS_BAD(ffd)) {
        pr_err("[FS]: %s: fd %d err!\n", __func__, fd);
        return -EINVAL;
    }
    if (!ffd->fsal->read)
        return -ENOSYS;
    /*
    int retval = -1;
    if (ffd->flags & FILE_FD_NORMAL) {
        retval = fsif.read(ffd->handle, buffer, nbytes);
    } else if (ffd->flags & FILE_FD_SOCKET) {
        retval = lwip_read(ffd->handle, buffer, nbytes);  
    } else if (ffd->flags & FILE_FD_DEVICE) {
        retval = device_read(ffd->handle, buffer, nbytes, ffd->offset);  
        if (retval > 0)
            ffd->offset += (retval / SECTOR_SIZE);
    } else if (ffd->flags & FILE_FD_FIFO) {
        retval = fifo_read(ffd->handle, buffer, nbytes, 0);  
    } else if ((ffd->flags & FILE_FD_PIPE0)) {
        retval = pipe_read(ffd->handle, buffer, nbytes);  
    }
    */
    return ffd->fsal->read(ffd->handle, buffer, nbytes);
}

int sys_write(int fd, void *buffer, size_t nbytes)
{
    if (fd < 0 || !nbytes || !buffer)
        return -EINVAL;
    #ifdef FSIF_USER_CHECK
    if (mem_copy_from_user(NULL, buffer, nbytes) < 0)
        return -EINVAL;
    #endif
    file_fd_t *ffd = fd_local_to_file(fd);
    if (FILE_FD_IS_BAD(ffd)) {
        pr_err("[FS]: %s: fd %d err! handle=%d flags=%x\n", __func__, 
            fd, ffd->handle, ffd->flags);
        return -EINVAL;
    }
    if (!ffd->fsal->write)
        return -ENOSYS;
    #if 0
    int retval = -1;
    
    if (ffd->flags & FILE_FD_NORMAL) {
        retval = fsif.write(ffd->handle, buffer, nbytes);
    } else if (ffd->flags & FILE_FD_SOCKET) {
        /* 由于lwip_write实现原因，需要内核缓冲区中转 */
        void *tmpbuffer = mem_alloc(nbytes);
        if (tmpbuffer == NULL)
            return -ENOMEM;
        memcpy(tmpbuffer, buffer, nbytes);
        retval = lwip_write(ffd->handle, tmpbuffer, nbytes);  
        mem_free(tmpbuffer);
    } else if (ffd->flags & FILE_FD_DEVICE) {
        retval = device_write(ffd->handle, buffer, nbytes, ffd->offset);  
        if (retval > 0)
            ffd->offset += (retval / SECTOR_SIZE);
    } else if (ffd->flags & FILE_FD_FIFO) {
        retval = fifo_write(ffd->handle, buffer, nbytes, 0);  
    } else if ((ffd->flags & FILE_FD_PIPE1)) {
        retval = pipe_write(ffd->handle, buffer, nbytes);  
    }
    #endif
    return ffd->fsal->write(ffd->handle, buffer, nbytes);
}

int sys_ioctl(int fd, int cmd, void *arg)
{
    file_fd_t *ffd = fd_local_to_file(fd);
    if (FILE_FD_IS_BAD(ffd))
        return -EINVAL;
    if (!ffd->fsal->ioctl)
        return -ENOSYS;
    #if 0
    if (ffd->flags & FILE_FD_NORMAL) {
        return fsif.ioctl(ffd->handle, cmd, arg);
    } else if (ffd->flags & FILE_FD_DEVICE) {
        return device_devctl(ffd->handle, cmd, arg);
    } else if (ffd->flags & FILE_FD_FIFO) {
        return fifo_ctl(ffd->handle, cmd, arg);
    } else if (ffd->flags & FILE_FD_PIPE0) {
        return pipe_ioctl(ffd->handle, cmd, arg, 0);
    } else if (ffd->flags & FILE_FD_PIPE1) {
        return pipe_ioctl(ffd->handle, cmd, arg, 1);
    }
    return -1;
    #endif
    return ffd->fsal->ioctl(ffd->handle, cmd, (unsigned long )arg);
}

int sys_fcntl(int fd, int cmd, long arg)
{
    file_fd_t *ffd = fd_local_to_file(fd);
    if (FILE_FD_IS_BAD(ffd))
        return -EINVAL;
    if (ffd->flags & FILE_FD_NORMAL) {
        return fsif.fcntl(ffd->handle, cmd, arg);
    } else if (ffd->flags & FILE_FD_SOCKET) {
        return lwip_fcntl(ffd->handle, cmd, arg);  
    } else if (ffd->flags & FILE_FD_PIPE0) {
        return pipe_ioctl(ffd->handle, cmd, arg, 0);
    } else if (ffd->flags & FILE_FD_PIPE1) {
        return pipe_ioctl(ffd->handle, cmd, arg, 1);
    }
    return -EINVAL;
}

int sys_lseek(int fd, off_t offset, int whence)
{
    file_fd_t *ffd = fd_local_to_file(fd);
    if (FILE_FD_IS_BAD(ffd))
        return -EINVAL;
    if (!ffd->fsal->lseek)
        return -ENOSYS;
    return ffd->fsal->lseek(ffd->handle, offset, whence);
    #if 0
    if (ffd->flags & FILE_FD_NORMAL) {
        return fsif.lseek(ffd->handle, offset, whence);
    } else if (ffd->flags & FILE_FD_DEVICE) {
        ffd->offset = offset;
        return 0;
    }
    return -EINVAL;
    #endif
}

int sys_access(const char *path, int mode)
{
    if (!path)
        return -EINVAL;
    if (mem_copy_from_user(NULL, (void *)path, MAX_PATH) < 0)
        return -EINVAL;
    //printk("%s: path: %s\n", __func__, path);
    return fsif.access(path, mode);
}

int sys_unlink(const char *path)
{
    if (!path)
        return -EINVAL;
    if (mem_copy_from_user(NULL, (void *) path, MAX_PATH) < 0)
        return -EINVAL;
    return fsif.unlink((char *) path);
}

int sys_ftruncate(int fd, off_t offset)
{
    file_fd_t *ffd = fd_local_to_file(fd);
    if (FILE_FD_IS_BAD(ffd))
        return -EINVAL;
    if (ffd->flags & FILE_FD_NORMAL) {
        return fsif.ftruncate(ffd->handle, offset);
    }
    return -EINVAL;
}

int sys_fsync(int fd)
{
    file_fd_t *ffd = fd_local_to_file(fd);
    if (FILE_FD_IS_BAD(ffd))
        return -EINVAL;
    if (!ffd->fsal->fsync)
        return -ENOSYS;
    return ffd->fsal->fsync(ffd->handle);
}

long sys_tell(int fd)
{
    file_fd_t *ffd = fd_local_to_file(fd);
    if (FILE_FD_IS_BAD(ffd))
        return -EINVAL;
    if (ffd->flags & FILE_FD_NORMAL) {
        return fsif.ftell(fd);
    }
    return -EINVAL;
}

int sys_stat(const char *path, struct stat *buf)
{
    if (!path || !buf)
        return -EINVAL;
    if (mem_copy_from_user(NULL, (void *) path, MAX_PATH) < 0)
        return -EINVAL;
    if (mem_copy_to_user(buf, NULL, sizeof(struct stat)) < 0)
        return -EINVAL;
    return fsif.state((char *) path, buf);
}

int sys_fstat(int fd, struct stat *buf)
{ 
    if (!buf)
        return -EINVAL;
    if (mem_copy_to_user(buf, NULL, sizeof(struct stat)) < 0)
        return -EINVAL;
    file_fd_t *ffd = fd_local_to_file(fd);
    if (FILE_FD_IS_BAD(ffd))
        return -EINVAL;
    if (ffd->flags & FILE_FD_NORMAL) {
        return fsif.fstat(ffd->handle, buf);
    }
    return -EINVAL;
}

int sys_chmod(const char *path, mode_t mode)
{
    if (!path)
        return -EINVAL;
    if (mem_copy_from_user(NULL, (void *) path, MAX_PATH) < 0)
        return -EINVAL;
    return fsif.chmod((char *) path, mode);
}

int sys_fchmod(int fd, mode_t mode)
{
    file_fd_t *ffd = fd_local_to_file(fd);
    if (FILE_FD_IS_BAD(ffd))
        return -EINVAL;
    if (ffd->flags & FILE_FD_NORMAL) {
        return fsif.fchmod(ffd->handle, mode);
    }
    return -EINVAL;
}

int sys_mkdir(const char *path, mode_t mode)
{
    if (!path)
        return -EINVAL;
    if (mem_copy_from_user(NULL, (void *) path, MAX_PATH) < 0)
        return -EINVAL;
    return fsif.mkdir((char *) path, mode);
}

int sys_rmdir(const char *path)
{
    if (!path)
        return -EINVAL;
    if (mem_copy_from_user(NULL, (void *) path, MAX_PATH) < 0)
        return -EINVAL;
    return fsif.rmdir((char *) path);
}

int sys_rename(const char *source, const char *target)
{
    if (!source || !target)
        return -EINVAL;
    if (mem_copy_from_user(NULL, (void *) source, MAX_PATH) < 0)
        return -EINVAL;
    if (mem_copy_from_user(NULL, (void *) target, MAX_PATH) < 0)
        return -EINVAL;
    return fsif.rename((char *) source, (char *) target);
}

int sys_chdir(const char *path)
{
    if (!path)
        return -EINVAL;
    if (mem_copy_from_user(NULL, (void *) path, MAX_PATH) < 0)
        return -EINVAL;

    task_t *cur = task_current;
    if (!cur->fileman)
        return -EINVAL;
    
    //printk("%s: path:%s\n", __func__, path);
    /* 打开目录 */
    dir_t dir = sys_opendir(path);
    if (dir < 0) {
        return -ENOFILE;
    }
    sys_closedir(dir);

    /* 保存路径 */
    int len = strlen(path);
    memset(cur->fileman->cwd, 0, MAX_PATH);
    memcpy(cur->fileman->cwd, path, min(len, MAX_PATH));
    return 0;
}

int sys_getcwd(char *buf, int bufsz)
{
    if (!buf)
        return -EINVAL;
    
    task_t *cur = task_current;
    if (!cur->fileman)
        return -EINVAL;
    
    if (mem_copy_to_user(buf, cur->fileman->cwd, min(bufsz, MAX_PATH)) < 0)
        return -EINVAL;
    return 0;
}

dir_t sys_opendir(const char *path)
{
    if (!path)
        return -EINVAL;
    if (mem_copy_from_user(NULL, (void *) path, MAX_PATH) < 0)
        return -EINVAL;
    return fsif.opendir((char *) path);
}
int sys_closedir(dir_t dir)
{
    return fsif.closedir(dir);
}

int sys_readdir(dir_t dir, struct dirent *dirent)
{
    if (!dirent)
        return -EINVAL;
    if (mem_copy_to_user(dirent, NULL, sizeof(struct dirent)) < 0)
        return -EINVAL;
    return fsif.readdir(dir, dirent);
}

int sys_rewinddir(dir_t dir)
{
    return fsif.rewinddir(dir);
}

int fsif_grow(int fd)
{
    file_fd_t *ffd = fd_local_to_file(fd);
    if (FILE_FD_IS_BAD(ffd))
        return -1;
    if (ffd->flags & FILE_FD_NORMAL) {
        
    } else if (ffd->flags & FILE_FD_DEVICE) {
        if (device_incref(ffd->handle) < 0)
            return -1;
        
    } else if (ffd->flags & FILE_FD_FIFO) {
        if (fifo_grow(ffd->handle) < 0)
            return -1;
    } else if (ffd->flags & FILE_FD_PIPE0) {
        if (pipe_grow(ffd->handle, 0) < 0)
            return -1;
    } else if (ffd->flags & FILE_FD_PIPE1) {
        if (pipe_grow(ffd->handle, 1) < 0)
            return -1;
    } else if (ffd->flags & FILE_FD_SOCKET) {
        
    }
    return 0;
}

int fsif_degrow(int fd)
{
    file_fd_t *ffd = fd_local_to_file(fd);
    if (FILE_FD_IS_BAD(ffd))
        return -1;
    if (ffd->flags & FILE_FD_NORMAL) {
        
    } else if (ffd->flags & FILE_FD_DEVICE) {
        if (device_decref(ffd->handle) < 0)
            return -1;
    }
    return 0;
}


/**
 * 复制一个文件描述符
 * @oldfd: 需要复制的fd
 * 
 * 复制fd，把对应的资源的引用计数+1，然后安装到一个最小的fd中。
 * 
 * 成功返回fd，失败返回-1
 */
int sys_dup(int oldfd)
{
    file_fd_t *ffd = fd_local_to_file(oldfd);
    if (FILE_FD_IS_BAD(ffd))
        return -EINVAL;
    int newfd = -1;
    if (!ffd->fsal->incref)
        return -ENOSYS;
    if (ffd->fsal->incref(ffd->handle) < 0)
        return -EINVAL;
    newfd = local_fd_install(ffd->handle, ffd->flags & FILE_FD_TYPE_MASK);
    return newfd;
}

/**
 * 复制一个文件描述符
 * @oldfd: 需要复制的fd
 * @newfd: 需要复制到的fd
 * 
 * 复制fd，把对应的资源的引用计数+1，然后安装到newfd中，如果newfd已经打开，则需要先关闭
 * 如果oldfd和newfd一样，则直接返回newfd
 * 
 * 成功返回newfd，失败返回-1
 */
int sys_dup2(int oldfd, int newfd)
{
    file_fd_t *ffd = fd_local_to_file(oldfd);
    if (FILE_FD_IS_BAD(ffd))
        return -EINVAL;
    if (oldfd == newfd) /* 一样则直接返回 */
        return newfd;
    /* 查看新fd，看是否已经打开，如果是，则先关闭。 */
    file_fd_t *newffd = fd_local_to_file(newfd);
    if (newffd != NULL && newffd->handle >= 0 && newffd->flags != 0) {
        if (sys_close(newfd) < 0)
            return -1;   
    }
    if (!ffd->fsal->incref)
        return -ENOSYS;
    if (ffd->fsal->incref(ffd->handle) < 0)
        return -EINVAL;    
    newfd = local_fd_install(ffd->handle, ffd->flags & FILE_FD_TYPE_MASK);
    return newfd;
}

int sys_pipe(int fd[2])
{
    if (!fd)
        return -EINVAL;
    if (mem_copy_to_user(fd, NULL, sizeof(int) * 2) < 0)
        return -EINVAL;
    pipe_t *pipe = create_pipe();
    if (pipe == NULL)
        return -EINVAL;
    /* read */
    int rfd = local_fd_install(pipe->id, FILE_FD_PIPE0);
    if (rfd < 0) {
        destroy_pipe(pipe);
        return -EAGAIN;
    }
    /* write */
    int wfd = local_fd_install(pipe->id, FILE_FD_PIPE1);
    if (wfd < 0) {
        local_fd_uninstall(rfd);
        destroy_pipe(pipe);
        return -EAGAIN;
    }
    fd[0] = rfd;
    fd[1] = wfd;
    return 0;
}

int sys_mount(
    char *source,         /* 需要挂载的资源 */
    char *target,         /* 挂载到的目标位置 */
    char *fstype,         /* 文件系统类型 */
    unsigned long mountflags    /* 挂载标志 */
) {
    if (!source || !target || !fstype)
        return -EINVAL;
    if (mem_copy_from_user(NULL, source, MAX_PATH) < 0)
        return -EINVAL;
    if (mem_copy_from_user(NULL, target, MAX_PATH) < 0)
        return -EINVAL;
    if (mem_copy_from_user(NULL, fstype, 32) < 0)
        return -EINVAL;
    
    return fsif.mount(source, target, fstype, mountflags);
}

int sys_unmount(char *path, unsigned long flags)
{
    if (!path)
        return -EINVAL;
    if (mem_copy_from_user(NULL, path, MAX_PATH) < 0)
        return -EINVAL;
    return fsif.unmount(path, flags);
}

int sys_mkfs(char *source,         /* 需要创建FS的设备 */
    char *fstype,         /* 文件系统类型 */
    unsigned long flags   /* 标志 */
) {
    if (!source || !fstype)
        return -EINVAL;
    if (mem_copy_from_user(NULL, source, MAX_PATH) < 0)
        return -EINVAL;
    if (mem_copy_from_user(NULL, fstype, 32) < 0)
        return -EINVAL;
    return fsif.mkfs(source, fstype, flags);
}

int sys_probe(const char *name, int flags, char *buf, size_t buflen)
{
    if (!name || !buf)
        return -EINVAL;
    if (mem_copy_from_user(NULL, (void *) name, 32) < 0)
        return -EINVAL;
    if (mem_copy_to_user(buf, NULL, buflen) < 0)
        return -EINVAL;

    if (flags & O_DEVEX) {
        return device_probe_unused(name, buf, buflen);
    } else {
        printk("%s: do not support probe!\n");
    }
    return -EINVAL;
}

int sys_opendev(const char *path, int flags)
{
    if (!path)
        return -EINVAL; 
    /*if (mem_copy_from_user(NULL, (void *)path, MAX_PATH) < 0)
        return -EINVAL;*/
    int handle;
    int fd = -1;
    handle = devif.open((void *)path, flags);
    if (handle < 0)
        return -ENODEV;
    fd = local_fd_install(handle, FILE_FD_DEVICE);
    return fd;
}
