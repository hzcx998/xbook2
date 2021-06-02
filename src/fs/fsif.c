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
#include <xbook/dir.h>
#include <sys/ipc.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/select.h>
#include <fcntl.h>

// #define DEBUG_FSIF

#define FSIF_USER_CHECK

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
    char abs_path[MAX_PATH] = {0};
    build_path(path, abs_path);
    int handle = fsif.open((void *)abs_path, flags);
    if (handle < 0)
        return -ENOFILE;
    return local_fd_install(handle, FILE_FD_NORMAL);
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
    return ffd->fsal->ioctl(ffd->handle, cmd, arg);
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
        
        break;
    case F_SETFL:
        /* TODO: set file flags */
        /* set flags real */
        if (!ffd->fsal->fcntl)
            return -ENOSYS;
        return ffd->fsal->fcntl(ffd->handle, cmd, arg);
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
    char abs_path[MAX_PATH] = {0};
    build_path(path, abs_path);
    return fsif.access(abs_path, mode);
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
    char abs_path[MAX_PATH] = {0};
    build_path(path, abs_path);
    return fsif.unlink((char *) abs_path);
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
    char abs_path[MAX_PATH] = {0};
    build_path(path, abs_path);
    return fsif.state((char *) abs_path, buf);
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
    char abs_path[MAX_PATH] = {0};
    build_path(path, abs_path);
    return fsif.chmod((char *) abs_path, mode);
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
    char abs_path[MAX_PATH] = {0};
    build_path(path, abs_path);
    return fsif.mkdir((char *) abs_path, mode);
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
    char abs_path[MAX_PATH] = {0};
    build_path(path, abs_path);
    return fsif.rmdir((char *) abs_path);
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
    char abs_source[MAX_PATH] = {0};
    build_path(source, abs_source);
    char abs_target[MAX_PATH] = {0};
    build_path(target, abs_target);
    return fsif.rename((char *) abs_source, (char *) abs_target);
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
    return task_set_cwd(cur, path);
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

int kfile_getcwd(char *buf, int bufsz)
{
    if (!buf)
        return -EINVAL;
    task_t *cur = task_current;
    if (!cur->fileman)
        return -EINVAL;
    memcpy(buf, cur->fileman->cwd, min((bufsz == 0) ? MAX_PATH : bufsz, MAX_PATH));
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
    char abs_path[MAX_PATH] = {0};
    build_path(path, abs_path);
    return fsif.opendir((char *) abs_path);
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
    char abs_source[MAX_PATH] = {0};
    build_path(source, abs_source);
    char abs_target[MAX_PATH] = {0};
    build_path(target, abs_target);
    return fsif.mount(abs_source, abs_target, fstype, mountflags);
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
    char abs_path[MAX_PATH] = {0};
    build_path(path, abs_path);
    return fsif.unmount(abs_path, flags);
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
    char abs_source[MAX_PATH] = {0};
    build_path(source, abs_source);
    return fsif.mkfs(abs_source, fstype, flags);
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

static int do_select_check_fds(int maxfdp, fd_set *readfds, fd_set *writefds, fd_set *exceptfds)
{
    /* 检测fd有效性 */
    int i;
    file_fd_t *ffd;
    for (i = 0; i < maxfdp; i++) {
        if (readfds) {
            if (FD_ISSET(i, readfds)) {
                ffd = fd_local_to_file(i);
                if (FILE_FD_IS_BAD(ffd)) {
                    errprint("[fs] %s: fd %d err! handle=%d flags=%x\n", __func__, 
                        i, ffd->handle, ffd->flags);
                    return -1;
                }
            }
        }
        if (writefds) {
            if (FD_ISSET(i, writefds)) {
                ffd = fd_local_to_file(i);
                if (FILE_FD_IS_BAD(ffd)) {
                    errprint("[fs] %s: fd %d err! handle=%d flags=%x\n", __func__, 
                        i, ffd->handle, ffd->flags);
                    return -1;
                }
            }
        }
        if (exceptfds) {
            if (FD_ISSET(i, exceptfds)) {
                ffd = fd_local_to_file(i);
                if (FILE_FD_IS_BAD(ffd)) {
                    errprint("[fs] %s: fd %d err! handle=%d flags=%x\n", __func__, 
                        i, ffd->handle, ffd->flags);
                    return -1;
                }
            }
        }
    }
    return 0;
}

void fd_set_or(fd_set *dst, fd_set *src, int maxfdp)
{
    int i;
    for (i = 0; i < maxfdp; i+=8) {
        dst->fd_bits[i / 8] |= src->fd_bits[i / 8];
    }
}

int fd_set_empty(fd_set *set, int maxfdp)
{
    int i;
    for (i = 0; i < maxfdp; i++) {
        if (FD_ISSET(i, set)) {
            return 0;
        }
    }
    return 1;
}

void fd_set_dump(fd_set *set, int maxfdp)
{
    dbgprint("====FD SET DUMP====");
    int i;
    for (i = 0; i < maxfdp; i++) {
        if (FD_ISSET(i, set)) {
            dbgprint("1");
        } else {
            dbgprint("0");
        }
    }
    dbgprint("\n");
}

static int do_select(int maxfdp, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
    struct timeval *timeout)
{
    if (do_select_check_fds(maxfdp, readfds, writefds, exceptfds) < 0) {
        return -EBADF;
    }
    #ifdef DEBUG_SELECT
    fd_set_dump(readfds, maxfdp);
    fd_set_dump(writefds, maxfdp);
    fd_set_dump(exceptfds, maxfdp);
    #endif
#ifdef CONFIG_NET
    #define SELECT_FDS_NR   4
#else
    #define SELECT_FDS_NR   3
#endif
    /* 0: normal, 1:pipe0, 2:pipe1, netif:3 */
    fd_set readfds_tab[SELECT_FDS_NR], writefds_tab[SELECT_FDS_NR], exceptfds_tab[SELECT_FDS_NR];
    int i;
    for (i = 0; i < SELECT_FDS_NR; i++) {
        FD_ZERO(&readfds_tab[i]);
        FD_ZERO(&writefds_tab[i]);
        FD_ZERO(&exceptfds_tab[i]);
    }
    int j;
    file_fd_t *ffd;
    for (j = 0; j < SELECT_FDS_NR; j++) {
        for (i = 0; i < maxfdp; i++) {
            if (readfds) {
                if (FD_ISSET(i, readfds)) {
                    ffd = fd_local_to_file(i);
                    switch (ffd->flags & FILE_FD_TYPE_MASK) {
                    case FILE_FD_NORMAL:
                        if (j != 0) 
                            break;
                        #ifdef DEBUG_SELECT
                        dbgprint("fd read set normal select %d: fd %d\n", j, i);
                        #endif
                        FD_SET(i, &readfds_tab[j]);
                        break;    
                    case FILE_FD_PIPE0:
                        if (j != 1) 
                            break;
                        FD_SET(i, &readfds_tab[j]);
                        break;
                    case FILE_FD_PIPE1:
                        if (j != 2) 
                            break;
                        FD_SET(i, &readfds_tab[j]);
                        break;
                    #ifdef CONFIG_NET
                    case FILE_FD_SOCKET:
                        if (j != 3) 
                            break;
                        #ifdef DEBUG_SELECT
                        dbgprint("fd read set net select %d: fd %d\n", j, i);
                        #endif
                        FD_SET(i, &readfds_tab[j]);
                        break;
                    #endif
                    default:
                        errprint("%s: unknown read fd %d file type\n", __func__, i);
                        return -EBADF;
                    }
                }
            }
            if (writefds) {
                if (FD_ISSET(i, writefds)) {
                    ffd = fd_local_to_file(i);
                    switch (ffd->flags & FILE_FD_TYPE_MASK) {
                    case FILE_FD_NORMAL:
                        if (j != 0) 
                            break;
                        #ifdef DEBUG_SELECT
                        dbgprint("fd write set normal select %d: fd %d\n", j, i);
                        #endif
                        FD_SET(i, &writefds_tab[j]);
                        break;    
                    case FILE_FD_PIPE0:
                        if (j != 1) 
                            break;
                        FD_SET(i, &writefds_tab[j]);
                        break;
                    case FILE_FD_PIPE1:
                        if (j != 2) 
                            break;
                        FD_SET(i, &writefds_tab[j]);
                        break;
                    #ifdef CONFIG_NET
                    case FILE_FD_SOCKET:
                        if (j != 3) 
                            break;
                        #ifdef DEBUG_SELECT
                        dbgprint("fd write set normal select %d: fd %d\n", j, i);
                        #endif
                        FD_SET(i, &writefds_tab[j]);
                        break;
                    #endif
                    default:
                        errprint("%s: unknown write fd %d file type\n", __func__, i);
                        return -EBADF;
                    }
                }
            }
            if (exceptfds) {
                if (FD_ISSET(i, exceptfds)) {
                    ffd = fd_local_to_file(i);
                    switch (ffd->flags & FILE_FD_TYPE_MASK) {
                    case FILE_FD_NORMAL:
                        if (j != 0) 
                            break;
                        #ifdef DEBUG_SELECT
                        dbgprint("fd except set normal select %d: fd %d\n", j, i);
                        #endif
                        FD_SET(i, &exceptfds_tab[j]);
                        break;    
                    case FILE_FD_PIPE0:
                        if (j != 1) 
                            break;
                        FD_SET(i, &exceptfds_tab[j]);
                        break;
                    case FILE_FD_PIPE1:
                        if (j != 2) 
                            break;
                        FD_SET(i, &exceptfds_tab[j]);
                        break;
                    #ifdef CONFIG_NET
                    case FILE_FD_SOCKET:
                        if (j != 3) 
                            break;
                        #ifdef DEBUG_SELECT
                        dbgprint("fd except set normal select %d: fd %d\n", j, i);
                        #endif
                        FD_SET(i, &exceptfds_tab[j]);
                        break;
                    #endif
                    default:
                        errprint("%s: unknown except fd %d file type\n", __func__, i);
                        return -EBADF;
                    }
                }
            }
        }
    }

    /* zero old set */
    if (readfds)
        FD_ZERO(readfds);
    if (writefds)
        FD_ZERO(writefds);
    if (exceptfds)
        FD_ZERO(exceptfds);

    select_t selects[SELECT_FDS_NR] = {
        fsif.select,
        pipeif_rd.select,
        pipeif_wr.select,
        #ifdef CONFIG_NET
        netif_fsal.select
        #endif
    };
    
    int total = 0;
    for (i = 0; i < SELECT_FDS_NR; i++) {
        #ifdef DEBUG_SELECT
        dbgprint("select %d/%d\n", i, SELECT_FDS_NR);         
        #endif
        if (selects[i] != NULL) {   /* 抽象层接口支持select才判断 */
            /* 没有要查看的集，则continue */
            if (fd_set_empty(&readfds_tab[i], maxfdp) && 
                fd_set_empty(&writefds_tab[i], maxfdp) &&
                fd_set_empty(&exceptfds_tab[i], maxfdp))
                continue;
            #ifdef DEBUG_SELECT
            fd_set_dump(&readfds_tab[i], maxfdp);
            fd_set_dump(&writefds_tab[i], maxfdp);
            fd_set_dump(&exceptfds_tab[i], maxfdp);
            #endif
            int ret = selects[i](maxfdp, &readfds_tab[i], &writefds_tab[i], &exceptfds_tab[i], timeout);    
            if (ret < 0) {
                return ret;
            }
            /* write back */
            if (readfds) {
                fd_set_or(readfds, &readfds_tab[i], maxfdp);
                #ifdef DEBUG_SELECT
                dbgprint("after select %d/%d readfds\n", i, SELECT_FDS_NR);        
                fd_set_dump(readfds, maxfdp);
                #endif
            }
            if (writefds) {
                fd_set_or(writefds, &writefds_tab[i], maxfdp);
                #ifdef DEBUG_SELECT
                dbgprint("after select %d/%d writefds\n", i, SELECT_FDS_NR);        
                fd_set_dump(writefds, maxfdp);
                #endif
            }
            if (exceptfds) {
                fd_set_or(exceptfds, &exceptfds_tab[i], maxfdp);
                #ifdef DEBUG_SELECT
                dbgprint("after select %d/%d exceptfds\n", i, SELECT_FDS_NR);        
                fd_set_dump(exceptfds, maxfdp);
                #endif
            }
            total += ret;
        }
    }
    return total;
}

int sys_select(int maxfdp, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
    struct timeval *timeout)
{
    #ifdef DEBUG_SELECT
    dbgprint("maxfdp: %d, readfds:%p, writefds:%p, execptfds:%p, timeout:%p\n",
        maxfdp, readfds, writefds, exceptfds, timeout);
    #endif
    if (maxfdp < 0 || maxfdp > LOCAL_FILE_OPEN_NR)
        return -EINVAL;
    fd_set __readfds, __writefds, __exceptfds;
    struct timeval __timeout;
    if (readfds) {
        if (mem_copy_from_user(&__readfds, readfds, sizeof(fd_set)) < 0) {
            return -EINVAL;
        }
    }
    if (writefds) {
        if (mem_copy_from_user(&__writefds, writefds, sizeof(fd_set)) < 0) {
            return -EINVAL;
        }
    }
    if (exceptfds) {
        if (mem_copy_from_user(&__exceptfds, exceptfds, sizeof(fd_set)) < 0) {
            return -EINVAL;
        }
    }
    if (timeout) {
        if (mem_copy_from_user(&__timeout, timeout, sizeof(struct timeval)) < 0) {
            return -EINVAL;
        }
    }
    int ret = do_select(maxfdp,
        readfds == NULL ? NULL: &__readfds,
        writefds == NULL ? NULL: &__writefds,
        exceptfds == NULL ? NULL: &__exceptfds, 
        timeout == NULL ? NULL: &__timeout);
    /* 回写返回值 */
    if (readfds) {
        if (mem_copy_to_user(readfds, &__readfds, sizeof(fd_set)) < 0) {
            return -EINVAL;
        }
    }
    if (writefds) {
        if (mem_copy_from_user(writefds, &__writefds, sizeof(fd_set)) < 0) {
            return -EINVAL;
        }
    }
    if (exceptfds) {
        if (mem_copy_from_user(exceptfds, &__exceptfds, sizeof(fd_set)) < 0) {
            return -EINVAL;
        }
    }
    return ret;
}
