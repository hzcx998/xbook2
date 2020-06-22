#include <fsal/dir.h>
#include <stdlib.h>
#include <string.h>

/* 目录表指针 */
fsal_dir_t *fsal_dir_table;

/**
 * init_fsal_dir_table - 初始化目录表
 * 
 * 分配目录表内存，并清空
 */
int init_fsal_dir_table()
{
    fsal_dir_table = malloc(FSAL_DIR_OPEN_NR * sizeof(fsal_dir_t));
    if (fsal_dir_table == NULL) 
        return -1;
    memset(fsal_dir_table, 0, FSAL_DIR_OPEN_NR * sizeof(fsal_dir_t));
    return 0;
}

/**
 * fsal_dir_alloc - 从表中分配一个目录
 * 
 */
fsal_dir_t *fsal_dir_alloc()
{
    int i;
    for (i = 0; i < FSAL_DIR_OPEN_NR; i++) {
        if (!fsal_dir_table[i].flags) {
            /* 清空目录表的内容 */
            memset(&fsal_dir_table[i], 0, sizeof(fsal_dir_t));
            
            /* 记录使用标志 */
            fsal_dir_table[i].flags = FSAL_DIR_USED;

            return &fsal_dir_table[i];
        }
    }
    return NULL;
}

/**
 * fsal_dir_free - 释放一个目录
 * 
 */
int fsal_dir_free(fsal_dir_t *dir)
{
    if (!dir->flags)
        return -1;
    dir->flags = 0;
    return 0;
}
