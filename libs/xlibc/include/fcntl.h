
#ifndef _XLIBC_FCNTL_H
#define _XLIBC_FCNTL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <sys/types.h>

#ifndef F_DUPFD
#define F_DUPFD 0
#endif

#ifndef F_GETFD
#define F_GETFD 1
#endif

#ifndef F_SETFD
#define F_SETFD 2
#endif

#ifndef F_GETFL
#define F_GETFL 3
#endif

#ifndef F_SETFL
#define F_SETFL 4
#endif

#if __LONG_MAX == 0x7fffffffL
#define F_GETLK 12
#define F_SETLK 13
#define F_SETLKW 14
#else
#define F_GETLK 5
#define F_SETLK 6
#define F_SETLKW 7
#endif

#define F_RDLCK 0
#define F_WRLCK 1
#define F_UNLCK 2

#ifndef FD_NCLOEXEC
#define FD_NCLOEXEC    0
#endif

#ifndef FD_CLOEXEC
#define FD_CLOEXEC 1
#endif

/* file open 文件打开 */
#define O_RDONLY    0x01
#define O_WRONLY    0x02
#define O_RDWR      0x04
#define O_CREAT     0x08
#define O_CREATE    O_CREAT
#define O_TRUNC     0x10
#define O_APPEND    0x20
#define O_EXEC      0x80
#define O_TEXT      0x100   // 文本模式打开        
#define O_BINARY    0x200   // 二进制模式打开
#define O_NONBLOCK  0x400   // 无阻塞
#define O_NOCTTY    0x800   // 不设置为控制tty
#define O_EXCL      0x1000  // 打开时文件一定要不存在才行
#define O_DIRECTORY 0x0200000

#define O_NOFOLLOW  0100000
#define O_CLOEXEC  02000000

#define AT_FDCWD (-100)
#define AT_SYMLINK_NOFOLLOW 0x100
#define AT_REMOVEDIR 0x200
#define AT_SYMLINK_FOLLOW 0x400
#define AT_EACCESS 0x200

struct flock {
	short l_type;
	short l_whence;
	off_t l_start;
	off_t l_len;
	pid_t l_pid;
};

int fcntl(int fd, int cmd, ...);
int creat(const char *, mode_t);

#if defined(_LARGEFILE64_SOURCE) || defined(_GNU_SOURCE)
#define F_GETLK64 F_GETLK
#define F_SETLK64 F_SETLK
#define F_SETLKW64 F_SETLKW
#define flock64 flock
#define open64 open
#define openat64 openat
#define creat64 creat
#define lockf64 lockf
#define posix_fadvise64 posix_fadvise
#define posix_fallocate64 posix_fallocate
#define off64_t off_t
#endif

#ifdef __cplusplus
}
#endif

#endif  /* _XLIBC_FCNTL_H */
