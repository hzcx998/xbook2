#include <fsal/fsal.h>
#include <xbook/fs.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <fsal/fd.h>

int kfile_open(const char *path, int flags)
{
    if (!path)
        return -EINVAL; 
    int handle;
    int fd = -1;
    handle = fsif.open((void *)path, flags);
    if (handle < 0)
        return -1;
    fd = local_fd_install(handle, FILE_FD_NORMAL);
    return fd;
}

int kfile_close(int fd)
{
    file_fd_t *ffd = fd_local_to_file(fd);
    if (ffd == NULL || ffd->handle < 0 || ffd->flags == 0)
        return -EINVAL;

    if (ffd->flags & FILE_FD_NORMAL) {
        if (fsif.close(ffd->handle) < 0)
            return -1;    
    }
    return local_fd_uninstall(fd);
}

int kfile_read(int fd, void *buffer, size_t nbytes)
{
    if (fd < 0 || !nbytes || !buffer)
        return -1;
    file_fd_t *ffd = fd_local_to_file(fd);
    if (ffd == NULL || ffd->handle < 0 || ffd->flags == 0) {
        pr_err("[FS]: %s: fd %d err!\n", __func__, fd);
        return -1;
    }
    int retval = -1;
    if (ffd->flags & FILE_FD_NORMAL) {
        retval = fsif.read(ffd->handle, buffer, nbytes);
    }
    return retval;
}

int kfile_stat(const char *path, struct stat *buf)
{
    if (!path || !buf)
        return -1;
    return fsif.state((char *) path, buf);
}

int kfile_access(const char *path, int mode)
{
    if (!path)
        return -EINVAL;
    return fsif.access(path, mode);
}

int kfile_lseek(int fd, off_t offset, int whence)
{
    file_fd_t *ffd = fd_local_to_file(fd);
    if (ffd == NULL || ffd->handle < 0 || ffd->flags == 0)
        return -EINVAL;
    if (ffd->flags & FILE_FD_NORMAL) {
        return fsif.lseek(ffd->handle, offset, whence);
    }
    return -EINVAL;
}