#ifndef _SYS_STAT_H
#define _SYS_STAT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>
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

/* FIXME: supported __S_ISUID and __S_ISGID  in kernel. */
#define	__S_ISUID	04000	/* Set user ID on execution.  */
#define	__S_ISGID	02000	/* Set group ID on execution.  */

#define	S_ISUID __S_ISUID	/* Set user ID on execution.  */
#define	S_ISGID	__S_ISGID	/* Set group ID on execution.  */

typedef struct stat {
    mode_t     st_mode;       //文件访问权限
    ino_t      st_ino;       //索引节点号
    dev_t      st_dev;        //文件使用的设备号
    dev_t      st_rdev;       //设备文件的设备号
    nlink_t    st_nlink;      //文件的硬连接数
    uid_t      st_uid;        //所有者用户识别号
    gid_t      st_gid;        //组识别号
    off_t      st_size;       //以字节为单位的文件容量
    time_t     st_atime;      //最后一次访问该文件的时间
    time_t     st_mtime;      //最后一次修改该文件的时间
    time_t     st_ctime;      //最后一次改变该文件状态的时间
    blksize_t  st_blksize;    //包含该文件的磁盘块的大小
    blkcnt_t   st_blocks;     //该文件所占的磁盘块
} stat_t;

struct kstat {
        uint64_t st_dev;
        uint64_t st_ino;
        mode_t st_mode;
        uint32_t st_nlink;
        uint32_t st_uid;
        uint32_t st_gid;
        uint64_t st_rdev;
        unsigned long __pad;
        off_t st_size;
        uint32_t st_blksize;
        int __pad2;
        uint64_t st_blocks;
        long st_atime_sec;
        long st_atime_nsec;
        long st_mtime_sec;
        long st_mtime_nsec;
        long st_ctime_sec;
        long st_ctime_nsec;
        unsigned __unused[2];
};

int stat(const char *path, struct stat *buf);
int fstat(int fd, struct kstat *buf);

int chmod(const char *path, mode_t mode);
int fchmod(int fd, mode_t mode);
mode_t umask(mode_t mask);

#ifdef __cplusplus
}
#endif

#endif  /* _SYS_STAT_H */
