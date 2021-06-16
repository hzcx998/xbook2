#ifndef _XBOOK_FSAL_CORE_H
#define _XBOOK_FSAL_CORE_H

/* File system abstraction layer (FSAL) 文件系统抽象层 */
#include <types.h>
#include <stddef.h>
#include <stdint.h>
#include <xbook/list.h>
#include <xbook/spinlock.h>
#include <sys/select.h>
#include <sys/time.h>

#define FS_MODEL_NAME  "fsal"

#define LOCAL_FILE_OPEN_NR  128

/* 当需要从用户态复制数据时，需要一个临时缓冲区，这指明了缓冲区的大小 */
#define FSIF_RW_BUF_SIZE    512

/* 当需要从用户态复制数据时，需要将数据分词多个块进行读写 */
#define FSIF_RW_CHUNK_SIZE  8192

typedef int (*select_t)(int , fd_set *, fd_set *, fd_set *, struct timeval *);

typedef struct {
    list_t list;                    /* 系统抽象的链表 */
    char *name;                     /* 文件系统抽象层名字 */
    char **subtable;                /* 子系统表 */
    int (*mkfs)(char *, char *, unsigned long );
    int (*mount)(char *, char *, char *, unsigned long );
    int (*unmount)(char *, char *, unsigned long );
    int (*open)(void *, int );
    int (*close)(int );
    int (*read)(int , void *, size_t );
    int (*write)(int , void *, size_t );
    int (*fastread)(int , void *, size_t );
    int (*fastwrite)(int , void *, size_t );
    int (*lseek)(int , off_t , int );
    int (*opendir)(char *);
    int (*closedir)(int);
    int (*readdir)(int , void *);
    int (*mkdir)(char *, mode_t);
    int (*unlink)(char *);
    int (*rename)(char *, char *);
    int (*ftruncate)(int , off_t);
    int (*fsync)(int );
    int (*state)(char *, void *);
    int (*chmod)(char *, mode_t);
    int (*fchmod)(int , mode_t);
    int (*utime)(char *, time_t, time_t);
    int (*feof)(int );
    int (*ferror)(int );
    off_t (*ftell)(int );
    size_t (*fsize)(int );
    int (*rewind)(int );
    int (*rewinddir)(int );
    int (*rmdir)(char *);
    int (*chdir)(char *);
    int (*ioctl)(int, int, void *);
    int (*fcntl)(int, int, long);
    int (*fstat)(int, void *);
    int (*access)(const char *, int);
    int (*incref)(int);
    int (*decref)(int);
    void *(*mmap)(int , void *, size_t, int, int, off_t);
    int (*fastio)(int, int, void *);
    select_t select;
    void *extention;
} fsal_t;

/* 文件抽象层接口 */
extern fsal_t fsif;
extern fsal_t pipeif_rd;
extern fsal_t pipeif_wr;
#ifdef CONFIG_NET
extern fsal_t netif_fsal;
#endif

int fsal_init();

#define INVALID_FD_TYPE(fd, type) (!((fd)->flags & type))

typedef struct {
    int handle;         /* 对象句柄 */
    uint32_t flags;     /* 对象的标志 */
    off_t offset;       /* 数据偏移 */
    fsal_t *fsal;       /* 文件操作集 */
} file_fd_t;

typedef struct {
    file_fd_t fds[LOCAL_FILE_OPEN_NR];
    char cwd[MAX_PATH];
    spinlock_t lock;
} file_man_t;

#define FILE_FD_IS_BAD(ffd) (!ffd || (ffd->handle < 0) || \
        (!ffd->flags) || (!ffd->fsal < 0))


#endif  /* _XBOOK_FSAL_CORE_H */