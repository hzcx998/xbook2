#ifndef __FILESRV_FSAL_DIR_H__
#define __FILESRV_FSAL_DIR_H__

#include "fsal.h"
#include <ff.h>     // fatfs 头文件
#include <stdint.h>

/* 允许打开的目录数量 */
#define FSAL_DIR_OPEN_NR       128

#define FSAL_DIR_USED      0X01 

/* 目录结构 */
typedef int dir_t;

typedef struct {
    char flags;             /* 文件标志 */
    fsal_t *fsal;           /* 文件系统抽象 */
    union {
        DIR fatfs;          /* fatfs目录结构 */
    } dir;    
} fsal_dir_t;

#define DIR_NAME_LEN    256

#define DE_RDONLY      0x01     /* read only */
#define DE_HIDDEN      0x02     /* hidden */
#define DE_SYSTEM      0x04     /* system */
#define DE_DIR         0x10     /* dir */
#define DE_ARCHIVE     0x20     /* archive */


typedef struct {
    size_t d_size;          /* 目录项大小 */
    uint32_t d_time;        /* 时间 */
    uint32_t d_date;        /* 日期 */
    mode_t d_attr;          /* 属性 */
    char d_name[DIR_NAME_LEN]; /* 名字 */
} dirent_t;

#define fstate_t dirent_t 

extern fsal_dir_t *fsal_dir_table;

/* 文件指针转换成在表中的索引 */
#define FSAL_D2I(dir)  ((int) ((dir) - fsal_dir_table))
/* 在表中的索引转换成文件指针 */
#define FSAL_I2D(idx)  ((fsal_dir_t *)(&fsal_dir_table[(idx)]))

#define ISBAD_FSAL_DIDX(idx) ((idx) < 0 || (idx) >= FSAL_DIR_OPEN_NR)

int init_fsal_dir_table();
fsal_dir_t *fsal_dir_alloc();
int fsal_dir_free(fsal_dir_t *dir);

#endif  /* __FILESRV_FSAL_DIR_H__ */