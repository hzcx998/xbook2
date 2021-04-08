#ifndef _SYS_STAT_H
#define _SYS_STAT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <types.h>

/* 高4位是属性位 */
#define S_IFSOCK 0x90    //scoket
#define S_IFLNK 0x50     //符号连接
#define S_IFIFO 0x30     //先进先出
#define S_IFBLK 0x80     //区块装置
#define S_IFCHR 0x40     //字符装置
#define S_IFDIR 0x20     //目录
#define S_IFREG 0x10     //一般文件

#define S_IREAD 0x04     //文件所有者具可读取权限
#define S_IWRITE 0x02    //文件所有者具可写入权限
#define S_IEXEC 0x01     //文件所有者具可执行权限

//上述的文件类型在POSIX中定义了检查这些类型的宏定义：
#define S_ISDIR(m)			((m) & S_IFDIR )    //是否为目录
#define S_ISCHR(m)			((m) & S_IFCHR )    //是否为字符设备
#define S_ISBLK(m)			((m) & S_IFBLK )    //是否为块设备
#define S_ISREG(m)			((m) & S_IFREG )    //是否为一般文件
#define S_ISLNK(m)			((m) & S_IFLNK )    //判断是否为符号连接
#define S_ISFIFO(m)			((m) & S_IFIFO )    //先进先出
#define S_ISSOCK(m)			((m) & S_IFSOCK )   //是否为socket

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

int stat(const char *path, struct stat *buf);
int fstat(int fd, struct stat *buf);
int chmod(const char *path, mode_t mode);
int fchmod(int fd, mode_t mode);
mode_t umask(mode_t mask);

#ifdef __cplusplus
}
#endif

#endif  /* _SYS_STAT_H */
