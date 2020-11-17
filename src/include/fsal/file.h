#ifndef _FSAL_FILE_H
#define _FSAL_FILE_H

/* File system abstraction layer (FSAL) 文件系统抽象层 */
#include "../../fs/fatfs/ff.h"
#include <types.h>
#include <stddef.h>
#include <stdint.h>
#include <xbook/list.h>
#include "fsal.h"

#define MT_REMKFS       0x01 /* 挂在前需要格式化磁盘 */
#define MT_DELAYED      0x02 /* 延时挂载 */

/* 允许打开的文件数量 */
#define FSAL_FILE_OPEN_NR       128
#define FSAL_FILE_FLAG_USED      0X01 

typedef struct {
    char flags;             /* 文件标志 */
    fsal_t *fsal;           /* 文件系统抽象 */
    union {
        FIL fatfs;          /* fatfs文件结构 */
    } file;    
} fsal_file_t;

extern fsal_file_t *fsal_file_table;

/* 文件指针转换成在表中的索引 */
#define FSAL_F2I(file)  ((int) ((file) - fsal_file_table))
/* 在表中的索引转换成文件指针 */
#define FSAL_I2F(idx)  ((fsal_file_t *)(&fsal_file_table[(idx)]))

#define FSAL_BAD_FIDX(idx) ((idx) < 0 || (idx) >= FSAL_FILE_OPEN_NR)

fsal_file_t *fsal_file_alloc();
int fsal_file_free(fsal_file_t *file);
int fsal_file_table_init();

#endif  /* _FSAL_FILE_H */