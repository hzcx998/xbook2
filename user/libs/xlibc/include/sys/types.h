#ifndef _SYS_TYPES_H
#define _SYS_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <types.h>

#define DIR_NAME_LEN    256

#define DE_RDONLY      0x01     /* read only */
#define DE_HIDDEN      0x02     /* hidden */
#define DE_SYSTEM      0x04     /* system */
#define DE_DIR         0x10     /* dir */
#define DE_ARCHIVE     0x20     /* archive */

typedef struct dirent {
    size_t d_size;          /* 目录项大小 */
    uint32_t d_time;        /* 时间 */
    uint32_t d_date;        /* 日期 */
    mode_t d_attr;          /* 属性 */
    char d_name[DIR_NAME_LEN]; /* 名字 */
} dirent_t;

typedef long ssize_t;

#ifdef __cplusplus
}
#endif

#endif  /* _SYS_TYPES_H */
