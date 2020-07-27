#include <fsal/fsal.h>
#include <xbook/fs.h>
#include <xbook/task.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

int sys_open(const char *path, int flags, int mode)
{
    int gfd = fsif.open((void *)path, flags);
    if (gfd < 0)
        return -1;
    return local_fd_install(gfd);
}

int sys_close(int fd)
{
    int gfd = fd_local_to_global(fd);
    if (gfd < 0)
        return -1;
    if (fsif.close(gfd) < 0)
        return -1;
    return local_fd_uninstall(fd);
}

int sys_read(int fd, void *buffer, size_t nbytes)
{
    int gfd = fd_local_to_global(fd);
    if (gfd < 0)
        return -1;
    return fsif.read(gfd, buffer, nbytes);
}

int sys_write(int fd, void *buffer, size_t nbytes)
{
    int gfd = fd_local_to_global(fd);
    if (gfd < 0)
        return -1;
    return fsif.write(gfd, buffer, nbytes);
}

int sys_ioctl(int fd, int cmd, unsigned long arg)
{
    int gfd = fd_local_to_global(fd);
    if (gfd < 0)
        return -1;
    return fsif.ioctl(gfd, cmd, arg);
}

int sys_fcntl(int fd, int cmd, long arg)
{
    int gfd = fd_local_to_global(fd);
    if (gfd < 0)
        return -1;
    return fsif.fcntl(gfd, cmd, arg);
}

int sys_lseek(int fd, off_t offset, int whence)
{
    int gfd = fd_local_to_global(fd);
    if (gfd < 0)
        return -1;
    return fsif.lseek(gfd, offset, whence);
}

int sys_access(const char *path, int mode)
{
    return fsif.access(path, mode);
}

int sys_unlink(const char *path)
{
    return fsif.unlink((char *) path);
}

int sys_ftruncate(int fd, off_t offset)
{
    return fsif.ftruncate(fd, offset);
}

int sys_fsync(int fd)
{
    return fsif.fsync(fd);
}

long sys_tell(int fd)
{
    return fsif.ftell(fd);
}

int sys_stat(const char *path, struct stat *buf)
{
    return fsif.state((char *) path, buf);
}

int sys_fstat(int fd, struct stat *buf)
{
    int gfd = fd_local_to_global(fd);
    if (gfd < 0)
        return -1;
    return fsif.fstat(gfd, buf);
}

int sys_chmod(const char *path, mode_t mode)
{
    return fsif.chmod((char *) path, mode);
}

int sys_fchmod(int fd, mode_t mode)
{
    int gfd = fd_local_to_global(fd);
    if (gfd < 0)
        return -1;
    return fsif.fchmod(gfd, mode);
}

int sys_mkdir(const char *path, mode_t mode)
{
    return fsif.mkdir((char *) path, mode);
}

int sys_rmdir(const char *path)
{
    return fsif.rmdir((char *) path);
}

int sys_rename(const char *source, const char *target)
{
    return fsif.rename((char *) source, (char *) target);
}

int sys_chdir(const char *path)
{
    task_t *cur = current_task;
    if (!cur->fileman)
        return -1;
    /* 只能改变到目录中 */
    struct stat buf;
    if (sys_stat(path, &buf) < 0)
        return -1;
    
    /* 检测是否为目录 */
    if (!S_ISDIR(buf.st_mode)) {
        return -1;  /* not dir */
    }

    /* 保存路径 */
    int len = strlen(path);
    memcpy(cur->fileman->cwd, path, min(len, MAX_PATH));
    return 0;
}

int sys_getcwd(char *buf, int bufsz)
{
    task_t *cur = current_task;
    if (!cur->fileman)
        return -1;
    memcpy(buf, cur->fileman->cwd, bufsz);
    return 0;
}

dir_t sys_opendir(const char *path)
{
    return fsif.opendir((char *) path);
}
int sys_closedir(dir_t dir)
{
    return fsif.closedir(dir);
}

int sys_readdir(dir_t dir, struct dirent *dirent)
{
    return fsif.readdir(dir, dirent);
}

int sys_rewinddir(dir_t dir)
{
    return fsif.rewinddir(dir);
}
