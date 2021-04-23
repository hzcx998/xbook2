#include <xbook/fsal.h>
#include <xbook/fs.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <xbook/fd.h>
#include <xbook/dir.h>

/* kfile 操作将会直接使用全局文件表，不会安装到用户的文件表中，因此可以长久存在 */

int kfile_open(const char *path, int flags)
{
    if (!path)
        return -EINVAL; 
    char abs_path[MAX_PATH] = {0};
    build_path(path, abs_path);
    return fsif.open((void *)abs_path, flags);
}

int kfile_close(int fd)
{
    if (!fsif.close)
        return -ENOSYS;
    if (fsif.close(fd) < 0)
        return -1;
    return 0;
}

int kfile_read(int fd, void *buffer, size_t nbytes)
{
    if (!nbytes || !buffer)
        return -1;
    if (!fsif.read)
        return -ENOSYS;
    return fsif.read(fd, buffer, nbytes);
}

int kfile_write(int fd, void *buffer, size_t nbytes)
{
    if (fd < 0 || !nbytes || !buffer)
        return -EINVAL;
    if (!fsif.write)
        return -ENOSYS;
    return fsif.write(fd, buffer, nbytes);
}

int kfile_stat(const char *path, struct stat *buf)
{
    if (!path || !buf)
        return -1;
    char abs_path[MAX_PATH] = {0};
    build_path(path, abs_path);
    return fsif.state((char *) abs_path, buf);
}

int kfile_access(const char *path, int mode)
{
    if (!path)
        return -EINVAL;
    char abs_path[MAX_PATH] = {0};
    build_path(path, abs_path);
    return fsif.access(abs_path, mode);
}

int kfile_lseek(int fd, off_t offset, int whence)
{
    if (!fsif.lseek)
        return -ENOSYS;
    return fsif.lseek(fd, offset, whence);
}

int kfile_ftell(int fd)
{
    if (!fsif.ftell)
        return -ENOSYS;
    return fsif.ftell(fd);
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
