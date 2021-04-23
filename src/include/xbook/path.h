#ifndef _XBOOK_FSAL_PATH_H
#define _XBOOK_FSAL_PATH_H

/* File system abstraction layer (FSAL) 文件系统抽象层 */

#include <types.h>
#include <stddef.h>
#include <stdint.h>
#include "fsal.h"

#define ROOT_DIR_PATH  "/root"
#define HOME_DIR_PATH  "/home"
#define ACCOUNT_DIR_PATH  "/acct"
#define DEV_DIR_PATH  "/dev"
#define FIFO_DIR_PATH  "/pipe"

/* #define RAMFS_DIR_PATH "/ramfs" */

#define MT_REMKFS       0x01 /* 挂在前需要格式化磁盘 */
#define MT_DELAYED      0x02 /* 延时挂载 */

/* 路径转换长度，一般是路径的前缀。例如/root, c: */
#define FASL_PATH_LEN   24

/* 路径转换表项数，决定最多可以支持的文件系统数量 */
#define FASL_PATH_NR   12

/* 路径转换 */
typedef struct {
    fsal_t *fsal;                   /* 文件系统抽象 */
    char path[FASL_PATH_LEN];       /* 具体文件系统的文件路径名 */
    char alpath[FASL_PATH_LEN];     /* 抽象层路径 */
    char devpath[FASL_PATH_LEN];    /* 设备路径 */
} fsal_path_t;

typedef enum {
    FSAL_PATH_TYPE_PHYSIC = 0,   /* 物理路径 */
    FSAL_PATH_TYPE_VIRTUAL,      /* 虚拟路径 */
    FSAL_PATH_TYPE_DEVICE,       /* 设备路径 */
    FSAL_PATH_TYPE_NR
} fsal_path_type_t;


extern fsal_path_t *fsal_path_table;

#define FSAL_PATH_TABLE_SIZE   (sizeof(fsal_path_t) * FASL_PATH_NR)

int fsal_path_init();
int fsal_path_insert(char *devpath, void *path, char *alpath, fsal_t *fsal);
int fsal_path_remove(void *path);
void fsal_path_print();
fsal_path_t *fsal_path_alloc();

fsal_path_t *fsal_path_find_with_type(fsal_path_type_t ptype, void *path, int inmaster);
#define fsal_path_find(alpath, inmaster)  fsal_path_find_with_type(FSAL_PATH_TYPE_VIRTUAL, (alpath), (inmaster))
#define fsal_path_find_device(devpath)  fsal_path_find_with_type(FSAL_PATH_TYPE_DEVICE, (devpath), 0)

int fsal_path_switch(fsal_path_t *fpath, char *new_path, char *old_path);
int fsal_list_dir(char* path);

char *path_get_filename(const char *path);

#endif  /* _XBOOK_FSAL_PATH_H */