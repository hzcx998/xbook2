#ifndef _XBOOK_FSAL_CORE_H
#define _XBOOK_FSAL_CORE_H

/* File system abstraction layer (FSAL) 文件系统抽象层 */
#include <types.h>
#include <stddef.h>
#include <stdint.h>
#include <xbook/list.h>
#include <xbook/spinlock.h>

#define FS_MODEL_NAME  "fsal"

#define LOCAL_FILE_OPEN_NR  128

typedef struct {
    list_t list;                    /* 系统抽象的链表 */
    char *name;                     /* 文件系统抽象层名字 */
    char **subtable;                /* 子系统表 */
    int (*mkfs)(char *, char *, unsigned long );
    int (*mount)(char *, char *, char *, unsigned long );
    int (*unmount)(char *, unsigned long );
    int (*open)(void *, int );
    int (*close)(int );
    int (*read)(int , void *, size_t );
    int (*write)(int , void *, size_t );
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
    int (*ioctl)(int, int, unsigned long);
    int (*fcntl)(int, int, long);
    int (*fstat)(int, void *);
    int (*access)(const char *, int);
    int (*incref)(int);
    int (*decref)(int);
    void *extention;
} fsal_t;

/* 文件抽象层接口 */
extern fsal_t fsif;
extern fsal_t devif;
extern fsal_t pipeif_rd;
extern fsal_t pipeif_wr;
extern fsal_t fifoif;

int fsal_init();

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