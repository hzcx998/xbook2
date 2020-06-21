#include <fsal/fsal.h>
#include <fsal/fatfs.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <filesrv.h>
#include <unistd.h>

/* 路径转换表 */
fsal_path_t *fsal_path_table;

/**
 * init_fsal_path_table - 初始化路径转换表
 * 
 * 分配路径表内存，并清空
 */
int init_fsal_path_table()
{
    fsal_path_table = malloc(FSAL_PATH_TABLE_SIZE);
    if (fsal_path_table == NULL) 
        return -1;
    memset(fsal_path_table, 0, FSAL_PATH_TABLE_SIZE);
    return 0;
}

int fsal_path_insert(void *path, char drive, fsal_t *fsal)
{
    char *p = (char *) path;

    /* 判断是否为正确的磁盘符 */
    if (FASL_BAD_DRIVE(drive))
        return -1;

    fsal_path_t *fpath = FSAL_I2P(FASL_DRV2I(drive));
    if (fpath->fsal) /* 不能挂载一个已经存在的磁盘符 */
        return -1;
    
    /* 填写内容 */
    fpath->fsal = fsal;
    strcpy(fpath->path, p); /* 复制路径 */
    fpath->path[strlen(path)] = '\0';
    return 0;
}

int fsal_path_remove(void *path)
{
    char *p = (char *) path;
    /* 比较是否已经在表中 */
    fsal_path_t *fpath;
    int i;
    for (i = 0; i < FASL_PATH_NR; i++) {
        fpath = &fsal_path_table[i];
        if (fpath->fsal) {
            if (!strcmp(p, fpath->path)) {
                /* 已经在表中，不能再插入 */
                fpath->fsal     = NULL;
                memset(fpath->path, 0, FASL_PATH_LEN);
                return 0;
            }
        }
    }
    return -1;
}

fsal_path_t *fsal_path_find(void *path)
{
    char *p = (char *) path;

    /* 判断是否为正确的磁盘符 */
    if (FASL_BAD_DRIVE(*p))
        return NULL;
    fsal_path_t *fpath = FSAL_I2P(FASL_DRV2I(*p));
    if (fpath->fsal) {
        return fpath;
    }
    return NULL;
}

/**
 * fsal_path_switch - 将抽象路径转换成具体路径
 * 
 */
int fsal_path_switch(fsal_path_t *fpath, char *new_path, char *old_path)
{
    if (!fpath || !new_path || !old_path)
        return -1;

    char *start = strchr(old_path, ':');
    if (start == NULL)
        return -1;

    start++;
    if (*start == 0) {   /* x:后面没有字符 */
        return -1;
    }

    /* 复制文件系统的挂载路径标识 */
    strcpy(new_path, fpath->path);
    /* 复制文件路径内容 */
    strcat(new_path, start);
    return 0;
}

void fsal_path_print()
{
    printf("%s: fsal path info:\n", SRV_NAME);
    fsal_path_t *fpath;
    int i;
    for (i = 0; i < FASL_PATH_NR; i++) {
        fpath = &fsal_path_table[i];
        if (fpath->fsal > 0) {
            printf("fasl path=%s fsal=%x\n", fpath->path, fpath->fsal);
        }
    }
}
