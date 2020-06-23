#ifndef _LIB_UNISTD_H
#define _LIB_UNISTD_H

#include "types.h"
#include "stddef.h"

/* file open 文件打开 */
#define O_RDONLY    0x01
#define O_WRONLY    0x02
#define O_RDWR      0x04
#define O_CREAT     0x08
#define O_TRUNC     0x10
#define O_APPEDN    0x20
#define O_EXEC      0x80
#define O_TEXT      0x100   // 文本模式打开        
#define O_BINARY    0x200   // 二进制模式打开

/* file seek */
#define SEEK_SET 1
#define SEEK_CUR 2
#define SEEK_END 3

/* file acesss 文件检测 */
#define R_OK 4 /* Test for read permission. */
#define W_OK 2 /* Test for write permission. */
#define X_OK 1 /* Test for execute permission. */
#define F_OK 0 /* Test for existence. */

#define STDIN_FILENO    0  /* 标准输入文件号 */
#define STDOUT_FILENO   1  /* 标准输出文件号 */
#define STDERR_FILENO   2  /* 标准错误文件号 */

int brk(void *addr);
void *sbrk(int increment);

int open(const char *path, int flags);
int close(int fd);
int read(int fd, void *buffer, size_t nbytes);
int write(int fd, void *buffer, size_t nbytes);
int lseek(int fd, off_t offset, int whence);
int access(const char *filenpath, int mode);
int unlink(const char *path);
int ftruncate(int fd, off_t offset);
int fsync(int fd);

int _eof(int fd);
int _error(int fd);
long tell(int fd);
size_t _size(int fd);
int rewind(int fd);

int execv(const char *path, const char *argv[]);
int execl(const char *path, const char *arg, ...);

#endif  /* _LIB_UNISTD_H */
