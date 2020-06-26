#ifndef _FILESRV_CORE_H
#define _FILESRV_CORE_H

/* 当前服务的名字 */
#define SRV_NAME    "filesrv"

#define SECTORS_PER_BLOCK   256

/* 原始磁盘，不在上面挂载文件系统 */
#define RAW_DISK    "ide0"

#define PATH_GUISRV "c:/sbin/guisrv"
#define PATH_NETSRV "c:/sbin/netsrv"

/* 文件映射 */
struct file_map {
    char *path;     /* 路径 */
    size_t size;    /* 实际大小 */
    off_t off;      /* 偏移 */
    char execute;   /* 是否需要执行 */
    const char **argv;
};



int init_srvcore();
int init_rom_file();
int init_child_proc();

#endif  /* _FILESRV_CORE_H */