#ifndef _SYS_STAT_H
#define _SYS_STAT_H

#include <stddef.h>
#include <stdint.h>
#include <types.h>

typedef struct stat {
    size_t      st_size;    /* 目录项大小 */
    uint32_t    st_time;    /* 时间 */
    uint32_t    st_date;    /* 日期 */
    mode_t      st_attr;    /* 属性 */
} stat_t;

int stat(const char *path, struct stat *buf);

int chmod(const char *path, mode_t mode);
int fchmod(int fd, mode_t mode);

#endif  /* _SYS_STAT_H */
