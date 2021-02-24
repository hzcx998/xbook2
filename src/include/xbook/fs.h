#ifndef _XBOOK_FS_H
#define	_XBOOK_FS_H

#include <stddef.h>
#include <stdint.h>
#include <types.h>
#include <sys/stat.h>
#include <xbook/spinlock.h>
#include <xbook/fsal.h>
#include <xbook/memspace.h>

int file_system_init();

int sys_open(const char *path, int flags);
int sys_close(int fd);
int sys_read(int fd, void *buffer, size_t nbytes);
int sys_write(int fd, void *buffer, size_t nbytes);
int sys_ioctl(int fd, int cmd, void *arg);
int sys_fcntl(int fd, int cmd, long arg);
int sys_lseek(int fd, off_t offset, int whence);
int sys_access(const char *path, int mode);
int sys_unlink(const char *path);
int sys_ftruncate(int fd, off_t offset);
int sys_fsync(int fd);
int sys_chmod(const char *path, mode_t mode);
int sys_fchmod(int fd, mode_t mode);
int sys_mkdir(const char *path, mode_t mode);
int sys_rmdir(const char *path);
int sys_rename(const char *source, const char *target);
int sys_chdir(const char *path);
int sys_getcwd(char *buf, int bufsz);
long sys_tell(int fd);
int sys_dup(int oldfd);
int sys_dup2(int oldfd, int newfd);
int sys_pipe(int fd[2]);
long sys_fsize(int fd);
void *sys_mmap(mmap_args_t *args);
int sys_fastio(int fd, int cmd, void *arg);
int sys_fastread(int fd, void *buffer, size_t nbytes);
int sys_fastwrite(int fd, void *buffer, size_t nbytes);

int sys_mount(
    char *source,         /* 需要挂载的资源 */
    char *target,         /* 挂载到的目标位置 */
    char *fstype,         /* 文件系统类型 */
    unsigned long mountflags    /* 挂载标志 */
);

int sys_unmount(char *path, unsigned long flags);
int sys_mkfs(char *source,         /* 需要创建FS的设备 */
    char *fstype,         /* 文件系统类型 */
    unsigned long flags   /* 标志 */
);

int sys_opendev(const char *path, int flags);
int sys_probedev(const char *name, char *buf, size_t buflen);
int sys_openfifo(const char *fifoname, int flags);

int fsif_incref(int fd);
int fsif_decref(int fd);

// special for kernel
int kfile_open(const char *path, int flags);
int kfile_read(int fd, void *buffer, size_t nbytes);
int kfile_write(int fd, void *buffer, size_t nbytes);
int kfile_stat(const char *path, struct stat *buf);
int kfile_access(const char *path, int mode);
int kfile_lseek(int fd, off_t offset, int whence);
int kfile_close(int fd);
int kfile_mkdir(const char *path, mode_t mode);
int kfile_rmdir(const char *path);

#endif /* _XBOOK_FS_H */