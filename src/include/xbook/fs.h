#ifndef _XBOOK_FS_H
#define	_XBOOK_FS_H

#define LOCAL_FILE_OPEN_NR  128

#include <stddef.h>
#include <stdint.h>
#include <types.h>
#include <sys/stat.h>


#define FILE_FD_ALLOC   0X01    /* alloced */
#define FILE_FD_NORMAL  0X02    /* is normal file */
#define FILE_FD_DEVICE  0X04    /* is a device */
#define FILE_FD_SOCKET  0X08    /* is a socket */
#define FILE_FD_FIFO    0X10    /* is a fifo */
#define FILE_FD_PIPE0   0X20    /* is a pipe0: read */
#define FILE_FD_PIPE1   0X40    /* is a pipe1: write */

typedef struct {
    int handle;         /* 对象句柄 */
    uint32_t flags;     /* 对象的标志 */
    off_t offset;       /* 数据偏移 */
} file_fd_t;

typedef struct {
    file_fd_t fds[LOCAL_FILE_OPEN_NR];
    char cwd[MAX_PATH];
} file_man_t;


int file_system_init();
/// syscall 
int sys_open(const char *path, int flags);
int sys_close(int fd);
int sys_read(int fd, void *buffer, size_t nbytes);
int sys_write(int fd, void *buffer, size_t nbytes);
int sys_ioctl(int fd, int cmd, unsigned long arg);
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

int sys_probe(const char *name, int flags, char *buf, size_t buflen);

int fsif_grow(int fd);
int fsif_degrow(int fd);

// special for kernel
int kern_file_open(const char *path, int flags);
int kern_file_read(int fd, void *buffer, size_t nbytes);
int kern_file_stat(const char *path, struct stat *buf);
int kern_file_access(const char *path, int mode);
int kern_file_lseek(int fd, off_t offset, int whence);
int kern_file_close(int fd);


#endif /* _XBOOK_FS_H */