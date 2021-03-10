#include <xbook/fsal.h>
#include <xbook/fd.h>
#include <xbook/fs.h>
#include <xbook/task.h>
#include <xbook/debug.h>
#include <xbook/driver.h>
#include <xbook/schedule.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <xbook/fifo.h>
#include <xbook/pipe.h>
#include <xbook/safety.h>
#include <xbook/account.h>
#include <sys/ipc.h>
#include <sys/ioctl.h>
#include <fcntl.h>

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
    if (!fsif.open)
        return -ENOSYS;
    
    if (!account_selfcheck_permission((char *)path, PERMISION_ATTR_FILE)) {
        return -EPERM;
    }

    int handle = fsif.open((void *)path, flags);
    if (handle < 0)
        return -ENOFILE;
    return local_fd_install(handle, FILE_FD_NORMAL);
}

int sys_openfifo(const char *fifoname, int flags)
{
    if (!fifoname)
        return -EINVAL; 
    #ifdef FSIF_USER_CHECK
    if (mem_copy_from_user(NULL, (void *)fifoname, MAX_PATH) < 0)
        return -EINVAL;
    #endif
    if (!fifoif.open)
        return -ENOSYS;
    if (!account_selfcheck_permission((char *)fifoname, PERMISION_ATTR_FIFO)) {
        return -EPERM;
    }
    int handle = fifoif.open((void *) fifoname, flags);
    if (handle < 0)
        return handle;
    return local_fd_install(handle, FILE_FD_FIFO);
}

int sys_opendev(const char *path, int flags)
{
    if (!path)
        return -EINVAL; 
    #ifdef FSIF_USER_CHECK
    if (mem_copy_from_user(NULL, (void *)path, MAX_PATH) < 0)
        return -EINVAL;
    #endif
    if (!devif.open) 
        return -ENOSYS;
    if (!account_selfcheck_permission((char *)path, PERMISION_ATTR_DEVICE)) {
        return -EPERM;
    }
    int handle;
    handle = devif.open((void *)path, flags);
    if (handle < 0)
        return -ENODEV;
    return local_fd_install(handle, FILE_FD_DEVICE);
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
        errprint("[FS]: %s: fd %d err!\n", __func__, fd);
        return -EINVAL;
    }
    if (!ffd->fsal->read)
        return -ENOSYS;
    return ffd->fsal->read(ffd->handle, buffer, nbytes);
}

int sys_fastread(int fd, void *buffer, size_t nbytes)
{
    if (fd < 0 || !nbytes || !buffer)
        return -EINVAL;
    #ifdef FSIF_USER_CHECK
    if (mem_copy_to_user(buffer, NULL, nbytes) < 0)
        return -EINVAL;
    #endif
    file_fd_t *ffd = fd_local_to_file(fd);
    if (FILE_FD_IS_BAD(ffd)) {
        errprint("[FS]: %s: fd %d err!\n", __func__, fd);
        return -EINVAL;
    }
    if (!ffd->fsal->fastread)
        return -ENOSYS;
    return ffd->fsal->fastread(ffd->handle, buffer, nbytes);
}

int sys_fastwrite(int fd, void *buffer, size_t nbytes)
{
    if (fd < 0 || !nbytes || !buffer)
        return -EINVAL;
    #ifdef FSIF_USER_CHECK
    if (mem_copy_to_user(buffer, NULL, nbytes) < 0)
        return -EINVAL;
    #endif
    file_fd_t *ffd = fd_local_to_file(fd);
    if (FILE_FD_IS_BAD(ffd)) {
        errprint("[FS]: %s: fd %d err!\n", __func__, fd);
        return -EINVAL;
    }
    if (!ffd->fsal->fastwrite)
        return -ENOSYS;
    return ffd->fsal->fastwrite(ffd->handle, buffer, nbytes);
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
        errprint("[FS]: %s: fd %d err! handle=%d flags=%x\n", __func__, 
            fd, ffd->handle, ffd->flags);
        return -EINVAL;
    }
    if (!ffd->fsal->write)
        return -ENOSYS;
    return ffd->fsal->write(ffd->handle, buffer, nbytes);
}

int sys_ioctl(int fd, int cmd, void *arg)
{
    file_fd_t *ffd = fd_local_to_file(fd);
    if (FILE_FD_IS_BAD(ffd))
        return -EINVAL;
    if (!ffd->fsal->ioctl)
        return -ENOSYS;
    return ffd->fsal->ioctl(ffd->handle, cmd, (unsigned long )arg);
}

int sys_fastio(int fd, int cmd, void *arg)
{
    file_fd_t *ffd = fd_local_to_file(fd);
    if (FILE_FD_IS_BAD(ffd))
        return -EINVAL;
    if (!ffd->fsal->fastio)
        return -ENOSYS;
    return ffd->fsal->fastio(ffd->handle, cmd, arg);
}

int sys_fcntl(int fd, int cmd, long arg)
{
    file_fd_t *ffd = fd_local_to_file(fd);
    if (FILE_FD_IS_BAD(ffd))
        return -EINVAL;
    int newfd = -1;
    switch (cmd) {
    case F_DUPFD: /* 复制一个基于arg（basefd）的最小的fd */
    {
        if (ffd->fsal->incref(ffd->handle) < 0)
            return -EINVAL;
        newfd = local_fd_install_based(ffd->handle, ffd->flags & FILE_FD_TYPE_MASK, arg);
        return newfd;
    }
    case F_GETFD:
        return (ffd->flags & FILE_FD_CLOEXEC) ? FD_CLOEXEC : FD_NCLOEXEC;
    case F_SETFD:
        if (arg & FD_CLOEXEC)
            ffd->flags |= FILE_FD_CLOEXEC;
        else
            ffd->flags &= ~FILE_FD_CLOEXEC;
        break;
    case F_GETFL:
        /* TODO: return file flags */
        return 0;
    case F_SETFL:
        /* TODO: set file flags */
        break;
    default:
        break;
    }
    return -1;
}

int sys_lseek(int fd, off_t offset, int whence)
{
    file_fd_t *ffd = fd_local_to_file(fd);
    if (FILE_FD_IS_BAD(ffd))
        return -EINVAL;
    if (!ffd->fsal->lseek)
        return -ENOSYS;
    return ffd->fsal->lseek(ffd->handle, offset, whence);
}

void *__sys_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset)
{
    file_fd_t *ffd = fd_local_to_file(fd);
    if (FILE_FD_IS_BAD(ffd))
        return NULL;
    if (!ffd->fsal->mmap)
        return NULL;
    return ffd->fsal->mmap(ffd->handle, addr, length, prot, flags, offset);
}

void *sys_mmap(mmap_args_t *args)
{
    return __sys_mmap(args->addr, args->length, args->prot, args->flags, args->fd, args->offset);
}

int sys_access(const char *path, int mode)
{
    if (!path)
        return -EINVAL;
    if (mem_copy_from_user(NULL, (void *)path, MAX_PATH) < 0)
        return -EINVAL;
    if (!account_selfcheck_permission((char *)path, PERMISION_ATTR_FILE)) {
        return -EPERM;
    }
    return fsif.access(path, mode);
}

int sys_unlink(const char *path)
{
    if (!path)
        return -EINVAL;
    if (mem_copy_from_user(NULL, (void *) path, MAX_PATH) < 0)
        return -EINVAL;
    if (!account_selfcheck_permission((char *)path, PERMISION_ATTR_FILE)) {
        return -EPERM;
    }
    return fsif.unlink((char *) path);
}

int sys_ftruncate(int fd, off_t offset)
{
    file_fd_t *ffd = fd_local_to_file(fd);
    if (FILE_FD_IS_BAD(ffd))
        return -EINVAL;
    if (!ffd->fsal->ftruncate)
        return -ENOSYS;
    return ffd->fsal->ftruncate(ffd->handle, offset);
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
    if (!ffd->fsal->ftell)
        return -ENOSYS;
    return ffd->fsal->ftell(ffd->handle);
}

long sys_fsize(int fd)
{
    file_fd_t *ffd = fd_local_to_file(fd);
    if (FILE_FD_IS_BAD(ffd))
        return -EINVAL;
    if (!ffd->fsal->fsize)
        return -ENOSYS;
    return ffd->fsal->fsize(ffd->handle);
}

int sys_stat(const char *path, struct stat *buf)
{
    if (!path || !buf)
        return -EINVAL;
    if (mem_copy_from_user(NULL, (void *) path, MAX_PATH) < 0)
        return -EINVAL;
    if (mem_copy_to_user(buf, NULL, sizeof(struct stat)) < 0)
        return -EINVAL;
    if (!account_selfcheck_permission((char *)path, PERMISION_ATTR_FILE)) {
        return -EPERM;
    }
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
    if (!ffd->fsal->fstat)
        return -ENOSYS;
    return ffd->fsal->fstat(ffd->handle, buf);
}

int sys_chmod(const char *path, mode_t mode)
{
    if (!path)
        return -EINVAL;
    if (mem_copy_from_user(NULL, (void *) path, MAX_PATH) < 0)
        return -EINVAL;
    if (!account_selfcheck_permission((char *)path, PERMISION_ATTR_FILE)) {
        return -EPERM;
    }
    return fsif.chmod((char *) path, mode);
}

int sys_fchmod(int fd, mode_t mode)
{
    file_fd_t *ffd = fd_local_to_file(fd);
    if (FILE_FD_IS_BAD(ffd))
        return -EINVAL;
    if (!ffd->fsal->fchmod)
        return -ENOSYS;
    return ffd->fsal->fchmod(ffd->handle, mode);
}

int sys_mkdir(const char *path, mode_t mode)
{
    if (!path)
        return -EINVAL;
    if (mem_copy_from_user(NULL, (void *) path, MAX_PATH) < 0)
        return -EINVAL;
    if (!account_selfcheck_permission((char *)path, PERMISION_ATTR_FILE)) {
        return -EPERM;
    }
    return fsif.mkdir((char *) path, mode);
}

int sys_rmdir(const char *path)
{
    if (!path)
        return -EINVAL;
    if (mem_copy_from_user(NULL, (void *) path, MAX_PATH) < 0)
        return -EINVAL;
    if (!account_selfcheck_permission((char *)path, PERMISION_ATTR_FILE)) {
        return -EPERM;
    }
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
    if (!account_selfcheck_permission((char *)source, PERMISION_ATTR_FILE)) {
        return -EPERM;
    }
    if (!account_selfcheck_permission((char *)target, PERMISION_ATTR_FILE)) {
        return -EPERM;
    }
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
    dir_t dir = sys_opendir(path);
    if (dir < 0) {
        return -ENOFILE;
    }
    sys_closedir(dir);
    task_set_cwd(cur, path);
    return 0;
}

int sys_getcwd(char *buf, int bufsz)
{
    if (!buf)
        return -EINVAL;
    task_t *cur = task_current;
    if (!cur->fileman)
        return -EINVAL;
    if (mem_copy_to_user(buf, cur->fileman->cwd, min((bufsz == 0) ? MAX_PATH : bufsz,
        MAX_PATH)) < 0)
        return -EINVAL;
    return 0;
}

dir_t sys_opendir(const char *path)
{
    if (!path)
        return -EINVAL;
    if (mem_copy_from_user(NULL, (void *) path, MAX_PATH) < 0)
        return -EINVAL;
    if (!account_selfcheck_permission((char *)path, PERMISION_ATTR_FILE)) {
        return -EPERM;
    }
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

int fsif_incref(int fd)
{
    file_fd_t *ffd = fd_local_to_file(fd);
    if (FILE_FD_IS_BAD(ffd))
        return -1;
    if (!ffd->fsal->incref)
        return -ENOSYS;
    return ffd->fsal->incref(ffd->handle);
}

int fsif_decref(int fd)
{
    file_fd_t *ffd = fd_local_to_file(fd);
    if (FILE_FD_IS_BAD(ffd))
        return -1;
    if (!ffd->fsal->decref)
        return -ENOSYS;
    return ffd->fsal->decref(ffd->handle);
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
    
    if (!account_selfcheck_permission((char *)source, PERMISION_ATTR_DEVICE)) {
        return -EPERM;
    }
    if (!account_selfcheck_permission((char *)target, PERMISION_ATTR_FILE)) {
        return -EPERM;
    }

    return fsif.mount(source, target, fstype, mountflags);
}

int sys_unmount(char *path, unsigned long flags)
{
    if (!path)
        return -EINVAL;
    if (mem_copy_from_user(NULL, path, MAX_PATH) < 0)
        return -EINVAL;
    if (!account_selfcheck_permission((char *)path, PERMISION_ATTR_FILE)) {
        return -EPERM;
    }
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
    if (!account_selfcheck_permission((char *)source, PERMISION_ATTR_DEVICE)) {
        return -EPERM;
    }
    return fsif.mkfs(source, fstype, flags);
}

int sys_probedev(const char *name, char *buf, size_t buflen)
{
    if (!name || !buf)
        return -EINVAL;
    if (mem_copy_from_user(NULL, (void *) name, 32) < 0)
        return -EINVAL;
    if (mem_copy_to_user(buf, NULL, buflen) < 0)
        return -EINVAL;
    return device_probe_unused(name, buf, buflen);
}