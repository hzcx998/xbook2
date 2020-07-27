#ifndef _XBOOK_FS_H
#define	_XBOOK_FS_H

#define LOCAL_FILE_OPEN_NR  128

#include <stddef.h>
#include <types.h>

typedef struct {
    int fds[LOCAL_FILE_OPEN_NR];
    char cwd[MAX_PATH];
} file_man_t;


int init_fs();
/// syscall 
int sys_open(const char *path, int flags, int mode);
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

#endif /* _XBOOK_FS_H */