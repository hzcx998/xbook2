#ifndef _LIB_DIRENT_H
#define _LIB_DIRENT_H

#include <types.h>

#define DIR_NAME_LEN    256

#define DE_RDONLY      0x01     /* read only */
#define DE_HIDDEN      0x02     /* hidden */
#define DE_SYSTEM      0x04     /* system */
#define DE_DIR         0x10     /* dir */
#define DE_ARCHIVE     0x20     /* archive */
#define DE_BLOCK        0x40     /* block */
#define DE_CHAR         0x80     /* char */

typedef int dir_t;

typedef struct dirent {
    size_t d_size;          /* 目录项大小 */
    uint32_t d_time;        /* 时间 */
    uint32_t d_date;        /* 日期 */
    mode_t d_attr;          /* 属性 */
    char d_name[DIR_NAME_LEN]; /* 名字 */
} dirent_t;

dir_t sys_opendir(const char *path);
int sys_closedir(dir_t dir);
int sys_readdir(dir_t dir, struct dirent *dirent);
int sys_rewinddir(dir_t dir);

enum {
    DT_UNKNOWN = 0,         //未知类型
#define DT_UNKNOWN DT_UNKNOWN
    DT_FIFO = 1,            //管道
#define DT_FIFO DT_FIFO
    DT_CHR = 2,             //字符设备
#define DT_CHR DT_CHR
    DT_DIR = 4,             //目录
#define DT_DIR DT_DIR
    DT_BLK = 6,             //块设备
#define DT_BLK DT_BLK
    DT_REG = 8,             //常规文件
#define DT_REG DT_REG
    DT_LNK = 10,            //符号链接
#define DT_LNK DT_LNK
    DT_SOCK = 12,           //套接字
#define DT_SOCK DT_SOCK
    DT_WHT = 14             //链接
#define DT_WHT DT_WHT
};

struct linux_dirent64 {
        uint64        d_ino;
        int64         d_off;
        unsigned short  d_reclen;
        unsigned char   d_type;
        char            d_name[];
};

int sys_getdents(int fd, void *dirp, unsigned long len);

#endif  /* _LIB_DIRENT_H */
