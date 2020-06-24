#ifndef _SYS_STAT_H
#define _SYS_STAT_H

#include <stddef.h>
#include <stdint.h>
#include <types.h>

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

int chmod(const char *path, mode_t mode);
int fchmod(int fd, mode_t mode);

#endif  /* _SYS_STAT_H */
