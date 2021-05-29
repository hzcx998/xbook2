#ifndef _LIB_UNISTD_H
#define _LIB_UNISTD_H

#include <types.h>
#include <stddef.h>
#include <xbook/config.h>

/*
 * st_mode flags
 */
#define         S_IFMT  0170000 /* type of file ，文件类型掩码*/
#define         S_IFREG 0100000 /* regular 普通文件*/
#define         S_IFBLK 0060000 /* block special 块设备文件*/
#define         S_IFDIR 0040000 /* directory 目录文件*/
#define         S_IFCHR 0020000 /* character special 字符设备文件*/
#define         S_IFIFO 0010000 /* fifo */
#define         S_IFNAM 0050000 /* special named file */
#if !defined(_M_XOUT)
#define         S_IFLNK 0120000 /* symbolic link 链接文件*/
#endif /* !defined(_M_XOUT) */
#define         S_ISREG(m)      (((m) & S_IFMT) == S_IFREG)
#define         S_ISBLK(m)      (((m) & S_IFMT) == S_IFBLK)
#define         S_ISDIR(m)      (((m) & S_IFMT) == S_IFDIR)
#define         S_ISCHR(m)      (((m) & S_IFMT) == S_IFCHR)
#define         S_ISFIFO(m)     (((m) & S_IFMT) == S_IFIFO)
#define         S_ISNAM(m)      (((m) & S_IFMT) == S_IFNAM)
#if !defined(_M_XOUT)
#define         S_ISLNK(m)      (((m) & S_IFMT) == S_IFLNK)
#endif /* !defined(_M_XOUT) */
#define         S_IREAD 0x04     //文件所有者具可读取权限
#define         S_IWRITE 0x02    //文件所有者具可写入权限
#define         S_IEXEC 0x01     //文件所有者具可执行权限

/* 文件模式 */
#define M_IREAD  (1 << 2)     //文件可读取权限
#define M_IWRITE (1 << 1)    //文件可写入权限
#define M_IEXEC  (1 << 0)     //文件可执行权限

/* file open 文件打开 */
#if defined(CONFIG_NEWSYSCALL)
#define O_RDONLY 0x000
#define O_WRONLY 0x001
#define O_RDWR 0x002 // 可读可写
//#define O_CREATE 0x200
#define O_CREATE 0x40
#define O_DIRECTORY 0x0200000

// xbook need
#define O_CREAT     O_CREATE
#define O_TRUNC     0x10
#define O_BINARY    0x200   // 二进制模式打开
#define O_NONBLOCK  0x400   // 无阻塞
#define O_EXCL      0x1000  // 打开时文件一定要不存在才行
#define O_APPEND    0x20

#else
#define O_RDONLY    0x01
#define O_WRONLY    0x02
#define O_RDWR      0x04
#define O_CREAT     0x08
#define O_TRUNC     0x10
#define O_APPEND    0x20
#define O_EXEC      0x80
#define O_TEXT      0x100   // 文本模式打开        
#define O_BINARY    0x200   // 二进制模式打开
#define O_NONBLOCK  0x400   // 无阻塞
#define O_EXCL      0x1000  // 打开时文件一定要不存在才行

#define O_DIRECTORY 0x0200000
#endif

#ifndef SEEK_SET
/* file seek */
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#endif

/* file acesss 文件检测 */
#define R_OK 4 /* Test for read permission. */
#define W_OK 2 /* Test for write permission. */
#define X_OK 1 /* Test for execute permission. */
#define F_OK 0 /* Test for existence. */

#define STDIN_FILENO    0  /* 标准输入文件号 */
#define STDOUT_FILENO   1  /* 标准输出文件号 */
#define STDERR_FILENO   2  /* 标准错误文件号 */

#endif  /* _LIB_UNISTD_H */
