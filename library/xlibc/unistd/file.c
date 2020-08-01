#include <unistd.h>
#include <types.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <sys/socket.h>
#include <sys/srvcall.h>
#include <srv/filesrv.h>
#include <sys/dir.h>
#include <sys/syscall.h>
#include <sys/stat.h>

int open(const char *path, int flags, int mode)
{
    if (path == NULL)
        return -1;
    char full_path[MAX_PATH] = {0};
    build_path(path, full_path);
    const char *p = (const char *) full_path;

    return syscall3(int, SYS_OPEN, p, flags, mode);
}

int close(int fd)
{
    return syscall1(int, SYS_CLOSE, fd);
}

int read(int fd, void *buffer, size_t nbytes)
{
    return syscall3(int, SYS_READ, fd, buffer, nbytes);
}

int write(int fd, void *buffer, size_t nbytes)
{
    return syscall3(int, SYS_WRITE, fd, buffer, nbytes);
}

int ioctl(int fd, int cmd, unsigned long arg)
{
    return syscall3(int, SYS_IOCTL, fd, cmd, arg);
}

int fcntl(int fd, int cmd, long arg)
{
    return syscall3(int, SYS_FCNTL, fd, cmd, arg);
}

int lseek(int fd, off_t offset, int whence)
{
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
    return syscall2(int, SYS_FTRUNCATE, fd, offset);
}

int fsync(int fd)
{
    return syscall1(int, SYS_FSYNC, fd);
}

int fchmod(int fd, mode_t mode)
{
    return syscall2(int, SYS_FCHMOD, fd, mode);
}

long tell(int fd)
{
    return syscall1(int, SYS_TELL, fd);
}

int fstat(int fd, struct stat *buf)
{
    return syscall2(int, SYS_FSTAT, fd, buf);
}

int dup(int fd)
{
    return syscall1(int, SYS_DUP, fd);
}

int dup2(int oldfd, int newfd)
{
    return syscall2(int, SYS_DUP2, oldfd, newfd);
}
