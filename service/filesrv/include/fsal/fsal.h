#ifndef __FILESRV_FSAL_H__
#define __FILESRV_FSAL_H__

/* File system abstraction layer (FSAL) 文件系统抽象层 */
#include <ff.h>
#include <types.h>
#include <wchar.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/list.h>

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
    void *extention;
} fsal_t;

/* 文件抽象层接口 */
extern fsal_t fsif;

int init_fsal();

#define MT_REMKFS       0x01 /* 挂在前需要格式化磁盘 */
#define MT_DELAYED      0x02 /* 延时挂载 */

/* 允许打开的文件数量 */
#define FSAL_FILE_OPEN_NR       128

#define FSAL_FILE_USED      0X01 

typedef struct _fsal_file {
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

#define ISBAD_FSAL_FIDX(idx) ((idx) < 0 || (idx) >= FSAL_FILE_OPEN_NR)

fsal_file_t *fsal_file_alloc();
int fsal_file_free(fsal_file_t *file);

/* 路径转换长度 */
#define FASL_PATH_LEN   24

/* 路径转换表项数
映射后的路径：a, b, c, ... l.
 */
#define FASL_PATH_NR   12

#define FASL_DRIVE_MIN   'a'
#define FASL_DRIVE_MAX   'l'

#define FASL_DRV2I(drive)  ((drive) - FASL_DRIVE_MIN)
#define FASL_I2DRV(idx)  ((idx) + FASL_DRIVE_MIN)

#define FASL_BAD_DRIVE(drive) \
        ((drive) < FASL_DRIVE_MIN || (drive) > FASL_DRIVE_MAX)   

/* 路径转换 */
typedef struct {
    fsal_t *fsal;           /* 文件系统抽象 */
    char path[FASL_PATH_LEN];
} fsal_path_t;

extern fsal_path_t *fsal_path_table;

#define FSAL_PATH_TABLE_SIZE   (sizeof(fsal_path_t) * FASL_PATH_NR)

/* 文件指针转换成在表中的索引 */
#define FSAL_P2I(path)  ((int) ((path) - fsal_path_table))
/* 在表中的索引转换成文件指针 */
#define FSAL_I2P(idx)  ((fsal_path_t *)(&fsal_path_table[(idx)]))

int init_fsal_path_table();
int fsal_path_insert(void *path, char drive, fsal_t *fsal);
int fsal_path_remove(void *path);
void fsal_path_print();

fsal_path_t *fsal_path_find(void *path);
int fsal_path_switch(fsal_path_t *fpath, char *new_path, char *old_path);

int fsal_list_dir(char* path);

#endif  /* __FILESRV_FSAL_H__ */