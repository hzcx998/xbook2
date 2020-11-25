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
    if (!ffd->fsal->close)
        return -ENOSYS;
    if (ffd->fsal->close(ffd->handle) < 0)
        return -1;
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
    if (!ffd->fsal->read)
        return -ENOSYS;
    return ffd->fsal->read(ffd->handle, buffer, nbytes);
}

int kfile_write(int fd, void *buffer, size_t nbytes)
{
    if (fd < 0 || !nbytes || !buffer)
        return -EINVAL;
    file_fd_t *ffd = fd_local_to_file(fd);
    if (FILE_FD_IS_BAD(ffd)) {
        pr_err("[FS]: %s: fd %d err! handle=%d flags=%x\n", __func__, 
            fd, ffd->handle, ffd->flags);
        return -EINVAL;
    }
    if (!ffd->fsal->write)
        return -ENOSYS;
    return ffd->fsal->write(ffd->handle, buffer, nbytes);
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
    if (!ffd->fsal->lseek)
        return -ENOSYS;
    return ffd->fsal->lseek(ffd->handle, offset, whence);
}

int kfile_mkdir(const char *path, mode_t mode)
{
    if (!path)
        return -EINVAL;
    return fsif.mkdir((char *) path, mode);
}

int kfile_rmdir(const char *path)
{
    if (!path)
        return -EINVAL;
    return fsif.rmdir((char *) path);
}
