#ifndef _FSAL_CORE_H
#define _FSAL_CORE_H

/* File system abstraction layer (FSAL) 文件系统抽象层 */
#include "../../fs/fatfs/ff.h"
#include <types.h>
#include <stddef.h>
#include <stdint.h>
#include <xbook/list.h>

#define ROOT_DISK_NAME  "disk1"
#define ROOT_DIR_PATH  "/root"

#define FS_MODEL_NAME  "fsal"

typedef struct {
    list_t list;                    /* 系统抽象的链表 */
    char *name;                     /* 文件系统抽象层名字 */
    char **subtable;                /* 子系统表 */
    int (*mkfs)(char *, char *, unsigned long );
    int (*mount)(char *, char *, char *, unsigned long );
    int (*unmount)(char *, unsigned long );
    int (*open)(void *, int );
    int (*close)(int );
    int (*read)(int , void *, size_t );
    int (*write)(int , void *, size_t );
    int (*lseek)(int , off_t , int );
    int (*opendir)(char *);
    int (*closedir)(int);
    int (*readdir)(int , void *);
    int (*mkdir)(char *, mode_t);
    int (*unlink)(char *);
    int (*rename)(char *, char *);
    int (*ftruncate)(int , off_t);
    int (*fsync)(int );
    int (*state)(char *, void *);
    int (*chmod)(char *, mode_t);
    int (*fchmod)(int , mode_t);
    int (*utime)(char *, time_t, time_t);
    int (*feof)(int );
    int (*ferror)(int );
    off_t (*ftell)(int );
    size_t (*fsize)(int );
    int (*rewind)(int );
    int (*rewinddir)(int );
    int (*rmdir)(char *);
    int (*chdir)(char *);
    int (*ioctl)(int, int, unsigned long);
    int (*fcntl)(int, int, long);
    int (*fstat)(int, void *);
    int (*access)(const char *, int);
    void *extention;
} fsal_t;

/* 文件抽象层接口 */
extern fsal_t fsif;

int fsal_init();

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

/* 路径转换长度，一般是路径的前缀。例如/root, c: */
#define FASL_PATH_LEN   24

/* 路径转换表项数，决定最多可以支持的文件系统数量 */
#define FASL_PATH_NR   12

/* 路径转换 */
typedef struct {
    fsal_t *fsal;                   /* 文件系统抽象 */
    char path[FASL_PATH_LEN];       /* 具体文件系统的文件路径名 */
    char alpath[FASL_PATH_LEN];     /* 抽象层路径 */
} fsal_path_t;

extern fsal_path_t *fsal_path_table;

#define FSAL_PATH_TABLE_SIZE   (sizeof(fsal_path_t) * FASL_PATH_NR)

int fsal_path_init();
int fsal_path_insert(void *path, char *alpath, fsal_t *fsal);
int fsal_path_remove(void *path);
void fsal_path_print();
fsal_path_t *fsal_path_alloc();

fsal_path_t *fsal_path_find(void *alpath, int inmaster);
int fsal_path_switch(fsal_path_t *fpath, char *new_path, char *old_path);

int fsal_list_dir(char* path);

#include <xbook/task.h>

int fs_fd_init(task_t *task);
int fs_fd_exit(task_t *task);
int local_fd_install(int resid, unsigned int flags);
int local_fd_uninstall(int local_fd);
int local_fd_install_to(int resid, int newfd, unsigned int flags);
file_fd_t *fd_local_to_file(int local_fd);
int handle_to_local_fd(int handle, unsigned int flags);
int fs_fd_copy(task_t *src, task_t *dest);
int fs_fd_reinit(task_t *cur);

#endif  /* _FSAL_CORE_H */