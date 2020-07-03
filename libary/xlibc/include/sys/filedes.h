#ifndef _SYS_FILEDES_H
#define _SYS_FILEDES_H

/* 任务可以打开的文件数量 */
#define _MAX_FILEDES_NR     32

#define _FILE_USING     0x01    /* 使用中 */
#define _FILE_NORMAL    0x02    /* 普通文件 */
#define _FILE_SOCKET    0x04    /* 套接字文件 */
#define _FILE_DEV       0x08    /* 设备文件 */

struct _filedes {
    int flags;      /* 文件描述符的标志 */
    int handle;     /* 句柄 */
};

extern struct _filedes __filedes_table[];

struct _filedes *__alloc_filedes();
void __free_filedes(struct _filedes *_fil);

#define _IS_BAD_FD(fd) ((fd) < 0 || (fd) >= _MAX_FILEDES_NR)
#define _FD_TO_FILE(fd) (__filedes_table + (fd))
#define _FILE_TO_FD(file) ((file) - __filedes_table)
#define _FILE_HANDLE(file) ((file)->handle)
#define _FILE_FLAGS(file) ((file)->flags)
#define _INVALID_FILE(file) (!(file)->flags)


#endif   /* _SYS_FILEDES_H */