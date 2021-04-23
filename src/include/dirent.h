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


#endif  /* _LIB_DIRENT_H */
