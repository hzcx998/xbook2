#ifndef _XBOOK_FSAL_DIR_H
#define _XBOOK_FSAL_DIR_H

#include "fsal.h"
#include <stdint.h>
#include <types.h>

/* 允许打开的目录数量 */
#define FSAL_DIR_OPEN_NR       128

#define FSAL_DIR_USED      0X01 

/* 目录结构 */
typedef int dir_t;

typedef struct {
    char flags;             /* 文件标志 */
    fsal_t *fsal;           /* 文件系统抽象 */
    void *extension;        /* 目录扩展 */
    #if 0
    union {
        DIR fatfs;          /* fatfs目录结构 */
    } dir;
    #endif    
} fsal_dir_t;

#define fstate_t dirent_t 

extern fsal_dir_t *fsal_dir_table;

/* 文件指针转换成在表中的索引 */
#define FSAL_D2I(dir)  ((int) ((dir) - fsal_dir_table))
/* 在表中的索引转换成文件指针 */
#define FSAL_I2D(idx)  ((fsal_dir_t *)(&fsal_dir_table[(idx)]))

#define FSAL_IS_BAD_DIR(idx) ((idx) < 0 || (idx) >= FSAL_DIR_OPEN_NR)

int fsal_dir_table_init();
fsal_dir_t *fsal_dir_alloc();
int fsal_dir_free(fsal_dir_t *dir);

void build_path(const char *path, char *out_path);
void wash_path(char *old_path, char *new_path);

#endif  /* _XBOOK_FSAL_DIR_H */