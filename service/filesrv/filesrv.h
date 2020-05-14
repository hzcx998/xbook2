#ifndef _FILESRV_H
#define _FILESRV_H

#include <sys/srvcall.h>
#include <ff.h>

/* 当前服务的名字 */
#define SRV_NAME    "filesrv"

typedef int (*filesrv_func_t) (srvarg_t *);


extern filesrv_func_t filesrv_call_table[];


/* 允许打开的文件数量 */
#define FILESRV_FILE_OPEN_NR       128

typedef struct _filesrv_file {
    FIL fil;                /* 文件结构 */
    char flags;             /* 文件标志 */
} filesrv_file_t;

int filesrv_file_table_init();
filesrv_file_t *filesrv_alloc_file();
int filesrv_free_file(filesrv_file_t *file);
int filesrv_free_file2(FIL *fil);

int filesrv_init_interface();

int filesrv_init();
int filesrv_create_files();
int filesrv_execute();

#endif  /* _FILESRV_H */