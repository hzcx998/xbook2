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


int brk(void *addr);
void *sbrk(int increment);

int open(const char *path, int flags);
int close(int fd);
int read(int fd, void *buffer, size_t nbytes);
int write(int fd, void *buffer, size_t nbytes);
int lseek(int fd, off_t offset, int whence);

int execv(const char *path, const char *argv[]);
int execl(const char *path, const char *arg, ...);

#endif  /* _LIB_UNISTD_H */
