#include <unistd.h>
#include <types.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#include <sys/dir.h>
#include <sys/syscall.h>
#include <sys/stat.h>

int open(const char *path, int flags, ...)
{
    /* arg3:... unused */
    if (path == NULL)
        return -1;
    char full_path[MAX_PATH] = {0};
    build_path(path, full_path);
    const char *p = (const char *) full_path;
    int retval = syscall2(int, SYS_OPEN, p, flags);
    if (retval < 0) {
        _set_errno(-retval);
        retval = -1;
    } else {
        _set_errno(0);
    }
    return retval;
}

int close(int fd)
{
    if (fd < 0)
        return -1;
    return syscall1(int, SYS_CLOSE, fd);
}

int read(int fd, void *buffer, size_t nbytes)
{
    if (fd < 0)
        return -1;
    return syscall3(int, SYS_READ, fd, buffer, nbytes);
}

int write(int fd, const void *buffer, size_t nbytes)
{
    if (fd < 0)
        return -1;
    return syscall3(int, SYS_WRITE, fd, buffer, nbytes);
}

int ioctl(int fd, int cmd, void *arg)
{
    if (fd < 0)
        return -1;
    return syscall3(int, SYS_IOCTL, fd, cmd, arg);
}

int fcntl(int fd, int cmd, ...)
{
    if (fd < 0)
        return -1;
    va_list argptr;
    va_start( argptr, cmd);
    long arg = va_arg( argptr, long);
    va_end( argptr);
    return syscall3(int, SYS_FCNTL, fd, cmd, arg);
}

int lseek(int fd, off_t offset, int whence)
{
    if (fd < 0)
        return -1;
    return syscall3(int, SYS_LSEEK, fd, offset, whence);
}

int access(const char *filenpath, int mode)
{
    if (filenpath == NULL)
        return -1;
    char full_path[MAX_PATH] = {0};
    build_path(filenpath, full_path);
    const char *p = (const char *) full_path;
    return syscall2(int, SYS_ACCESS, p, mode);
}

int unlink(const char *path)
{
    if (path == NULL)
        return -1;
    char full_path[MAX_PATH] = {0};
    build_path(path, full_path);
    const char *p = (const char *) full_path;
    return syscall1(int, SYS_UNLINK, p);
}

int ftruncate(int fd, off_t offset)
{
    if (fd < 0)
        return -1;
    return syscall2(int, SYS_FTRUNCATE, fd, offset);
}

int fsync(int fd)
{
    if (fd < 0)
        return -1;
    return syscall1(int, SYS_FSYNC, fd);
}

int fchmod(int fd, mode_t mode)
{
    if (fd < 0)
        return -1;
    return syscall2(int, SYS_FCHMOD, fd, mode);
}

long tell(int fd)
{
    if (fd < 0)
        return -1;
    return syscall1(int, SYS_TELL, fd);
}

int fstat(int fd, struct stat *buf)
{
    if (fd < 0)
        return -1;
    return syscall2(int, SYS_FSTAT, fd, buf);
}

int dup(int fd)
{
    if (fd < 0)
        return -1;
    return syscall1(int, SYS_DUP, fd);
}

int dup2(int oldfd, int newfd)
{
    if (oldfd < 0 || newfd < 0)
        return -1;
    return syscall2(int, SYS_DUP2, oldfd, newfd);
}

int pipe(int fd[2])
{
    return syscall1(int, SYS_PIPE, fd);
}

int probedev(const char *name, char *buf, size_t buflen)
{
    return syscall3(int, SYS_PROBEDEV, name, buf, buflen);
}

int mkfifo(const char *fifoname, mode_t mode)
{
    return syscall2(int, SYS_MKFIFO, fifoname, mode);
}