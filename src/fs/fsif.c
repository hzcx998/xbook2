#include <fsal/fsal.h>
#include <xbook/fs.h>
#include <xbook/task.h>
#include <xbook/debug.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <lwip/sockets.h>

#define DEBUG_LOCAL 0

int sys_open(const char *path, int flags, int mode)
{
    //printk("[fs]: %s: path=%s flags=%x\n", __func__, path, flags);
    int fileres = fsif.open((void *)path, flags);
    if (fileres < 0)
        return -1;
    return local_fd_install(fileres, FILE_FD_NORMAL);
}

int sys_close(int fd)
{
    file_fd_t *ffd = fd_local_to_file(fd);
    if (ffd == NULL || ffd->handle < 0 || ffd->flags == 0)
        return -1;
    if (ffd->flags & FILE_FD_NORMAL) {
        if (fsif.close(ffd->handle) < 0)
            return -1;    
    } else if (ffd->flags & FILE_FD_SOCKET) {
        #if DEBUG_LOCAL == 1
        printk("[FS]: %s: close fd %d socket %d.\n", __func__, fd, ffd->handle);
        #endif
        if (lwip_close(ffd->handle) < 0)
            return -1;
    }
    return local_fd_uninstall(fd);
}

int sys_read(int fd, void *buffer, size_t nbytes)
{
    //printk("[fs]: %s: fd=%d buf=%x bytes=%d\n", __func__, fd, buffer, nbytes);
    file_fd_t *ffd = fd_local_to_file(fd);
    if (ffd == NULL || ffd->handle < 0 || ffd->flags == 0)
        return -1;
    if (ffd->flags & FILE_FD_NORMAL) {
        return fsif.read(ffd->handle, buffer, nbytes);
    } else if (ffd->flags & FILE_FD_SOCKET) {
        return lwip_read(ffd->handle, buffer, nbytes);  
    }
    return -1;
}

int sys_write(int fd, void *buffer, size_t nbytes)
{
    file_fd_t *ffd = fd_local_to_file(fd);
    if (ffd == NULL || ffd->handle < 0 || ffd->flags == 0)
        return -1;

    if (ffd->flags & FILE_FD_NORMAL) {
        return fsif.write(ffd->handle, buffer, nbytes);
    } else if (ffd->flags & FILE_FD_SOCKET) {
        /* 由于lwip_write实现原因，需要内核缓冲区中转 */
        void *tmpbuffer = kmalloc(nbytes);
        if (tmpbuffer == NULL)
            return -1;
        memcpy(tmpbuffer, buffer, nbytes);
        int wr = lwip_write(ffd->handle, tmpbuffer, nbytes);  
        kfree(tmpbuffer);
        return wr;
    }
    return -1;
}

int sys_ioctl(int fd, int cmd, unsigned long arg)
{
    file_fd_t *ffd = fd_local_to_file(fd);
    if (ffd == NULL || ffd->handle < 0 || ffd->flags == 0)
        return -1;
    if (ffd->flags & FILE_FD_NORMAL) {
        return fsif.ioctl(ffd->handle, cmd, arg);
    }
    return -1;
}

int sys_fcntl(int fd, int cmd, long arg)
{
    file_fd_t *ffd = fd_local_to_file(fd);
    if (ffd == NULL || ffd->handle < 0 || ffd->flags == 0)
        return -1;
    if (ffd->flags & FILE_FD_NORMAL) {
        return fsif.fcntl(ffd->handle, cmd, arg);
    } else if (ffd->flags & FILE_FD_SOCKET) {
        return lwip_fcntl(ffd->handle, cmd, arg);  
    }
    return -1;
}

int sys_lseek(int fd, off_t offset, int whence)
{
    file_fd_t *ffd = fd_local_to_file(fd);
    if (ffd == NULL || ffd->handle < 0 || ffd->flags == 0)
        return -1;
    if (ffd->flags & FILE_FD_NORMAL) {
        return fsif.lseek(ffd->handle, offset, whence);
    }
    return -1;
}

int sys_access(const char *path, int mode)
{
    //printk("%s: path: %s\n", __func__, path);
    return fsif.access(path, mode);
}

int sys_unlink(const char *path)
{
    return fsif.unlink((char *) path);
}

int sys_ftruncate(int fd, off_t offset)
{
    file_fd_t *ffd = fd_local_to_file(fd);
    if (ffd == NULL || ffd->handle < 0 || ffd->flags == 0)
        return -1;
    if (ffd->flags & FILE_FD_NORMAL) {
        return fsif.ftruncate(ffd->handle, offset);
    }
    return -1;
}

int sys_fsync(int fd)
{
    file_fd_t *ffd = fd_local_to_file(fd);
    if (ffd == NULL || ffd->handle < 0 || ffd->flags == 0)
        return -1;
    if (ffd->flags & FILE_FD_NORMAL) {
        return fsif.fsync(fd);
    }
    return -1;
}

long sys_tell(int fd)
{
    file_fd_t *ffd = fd_local_to_file(fd);
    if (ffd == NULL || ffd->handle < 0 || ffd->flags == 0)
        return -1;
    if (ffd->flags & FILE_FD_NORMAL) {
        return fsif.ftell(fd);
    }
    return -1;
}

int sys_stat(const char *path, struct stat *buf)
{
    return fsif.state((char *) path, buf);
}

int sys_fstat(int fd, struct stat *buf)
{
    file_fd_t *ffd = fd_local_to_file(fd);
    if (ffd == NULL || ffd->handle < 0 || ffd->flags == 0)
        return -1;
    if (ffd->flags & FILE_FD_NORMAL) {
        return fsif.fstat(ffd->handle, buf);
    }
    return -1;
}

int sys_chmod(const char *path, mode_t mode)
{
    return fsif.chmod((char *) path, mode);
}

int sys_fchmod(int fd, mode_t mode)
{
    file_fd_t *ffd = fd_local_to_file(fd);
    if (ffd == NULL || ffd->handle < 0 || ffd->flags == 0)
        return -1;
    if (ffd->flags & FILE_FD_NORMAL) {
        return fsif.fchmod(ffd->handle, mode);
    }
    return -1;
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
    
    //printk("%s: path:%s\n", __func__, path);
    /* 打开目录 */
    dir_t dir = sys_opendir(path);
    if (dir < 0) {
        return -1;
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
    task_t *cur = current_task;
    if (!cur->fileman)
        return -1;
    memcpy(buf, cur->fileman->cwd, min(bufsz, MAX_PATH));
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
