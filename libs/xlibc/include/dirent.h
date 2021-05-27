#ifndef _LIB_DIRENT_H
#define _LIB_DIRENT_H
#ifdef __cplusplus
extern "C" {
#endif
#include <sys/types.h>
#include <stdint.h>

typedef struct _dirdes {
    int flags;      /* 文件描述符的标志 */
    dir_t diridx;    /* 文件索引 */
} DIR;

DIR *opendir(const char *path);
int closedir(DIR *dir);
struct dirent *readdir(DIR *dir);
int rewinddir(DIR *dir);


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

int getdents(int fd, struct linux_dirent64 *dirp64, unsigned long len);

#ifdef __cplusplus
}
#endif

#endif  /* _LIB_DIRENT_H */
