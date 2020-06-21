#include <sys/res.h>
#include <sys/vmm.h>
#include <sys/proc.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/trigger.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/kfile.h>
#include <sys/ipc.h>
#include <sys/proc.h>
#include <ff.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <fsal/fsal.h>

#include "fatfs.h"
#include <filesrv.h>

#define DEBUG_LOCAL 0

/* 最多可以挂载的驱动器 */
#define FILESRV_DRV_NR   10

#define SECTORS_PER_BLOCK   256

/* 原始磁盘，不在上面挂载文件系统 */
#define RAW_DISK    "ide0"

int filesrv_create_file(char *path, size_t size, char *diskname, long off)
{
    int fi;
    int writebytes;

    long sectors = size / SECTOR_SIZE + 1;
    unsigned char *buf = malloc(sectors * SECTOR_SIZE);
    if (buf == NULL) {
        printf("%s: %s: malloc failed!\n", SRV_NAME, __func__);
        return -1;
    }
    memset(buf, 0, size);
    //printf("%s: %s: malloc ok.\n", SRV_NAME, __func__);
    
    fi = fsif.open(path, O_CREAT | O_RDWR);
    if (fi < 0) {
        printf("%s: %s: open file failed!\n", SRV_NAME, __func__);
        goto free_buf;
    }
    //printf("%s: %s: f_open ok.\n", SRV_NAME, __func__);
    
    /* open disk */
    int disk = res_open(diskname, RES_DEV, 0);
    if (disk < 0) {
        printf("%s: %s: open disk '%s' %d failed! exit now.",
            SRV_NAME, __func__, diskname);
        goto close_file;
    }

    /* 从磁盘读取文件 */
    int left_count = sectors;
    
    /* 小于1个块 */
    if (left_count < SECTORS_PER_BLOCK) {
        if (res_read(disk, off, buf, left_count * SECTOR_SIZE) < 0) {
            printf("%s: %s: open disk '%s' %d failed! exit now.", diskname, disk);
            res_close(disk);
            goto close_file;
        }
    } else {
        /* 处理小于DATA_BLOCK个块 */
        int chunk = left_count & 0xff;   /* 取256以下的数据数量 */
        int lba = off;
        unsigned char *p = buf;
        while (left_count > 0) {
            //printf("read at %d about %d\n", lba, chunk);
            if (res_read(disk, lba, p, chunk * SECTOR_SIZE) < 0) {
                printf("%s: %s: open disk '%s' %d failed! exit now.", diskname, disk);
                goto close_file;
            }
            lba += chunk;
            left_count -= chunk;
            p += chunk * SECTOR_SIZE;
            /* 每次处理BLOCK个 */
            chunk = SECTORS_PER_BLOCK;
        }
    }
    
    writebytes = fsif.write(fi, buf, size);
    if (writebytes != size) {
        printf("%s: %s: f_write failed!\n", SRV_NAME, __func__);
    }
    fsif.close(fi);
    free(buf);
    return 0;
close_file:
    fsif.close(fi);
free_buf:
    free(buf);

    return -1;
}

#define PATH_GUISRV "c:/guisrv"
#define PATH_NETSRV "c:/netsrv"

/* 文件映射 */
struct file_map {
    char *path;     /* 路径 */
    size_t size;    /* 实际大小 */
    off_t off;      /* 偏移 */
    char execute;   /* 是否需要执行 */
    const char **argv;
};

#define ARRAY_SIZE(array) (sizeof(array) / sizeof((array)[0]))

const char *infones_argv[3] = {
    "c:/infones",
    "c:/mario.nes",
    0
};

struct file_map file_map_table[] = {
    {PATH_GUISRV, 200 * 512, 800, 1, NULL},
    {PATH_NETSRV, 400 * 512, 1500, 0, NULL},
    {"c:/terminal", 200 * 512, 5100, 0, NULL},
//    {"/login", 100 * 512, 4000, 0, NULL},
//    {"/bosh", 100 * 512, 4100, 0, NULL},
    {"c:/test", 100 * 512, 4300, 0, NULL},
    {"c:/infones", 650 * 512, 4400, 0, infones_argv},
//    {"/mario.nes", 100 * 512, 10000, 0, NULL},
};

int filesrv_create_files()
{
    struct file_map *fmap;
    int i;
    for (i = 0; i < ARRAY_SIZE(file_map_table); i++) {
        fmap = &file_map_table[i];
        if (filesrv_create_file(fmap->path, fmap->size, RAW_DISK, fmap->off)) {
            continue;
        }
    }
    return 0;
}

int filesrv_execute()
{
    struct file_map *fmap;
    int pid = 0;
    int i;
    for (i = 0; i < ARRAY_SIZE(file_map_table); i++) {
        fmap = &file_map_table[i];
        if (fmap->execute) {
            pid = fork();
            if (pid < 0) {
                printf("%s: %s: fork failed!\n", SRV_NAME, __func__);
                return -1;
            }
            if (!pid) { /* 子进程执行新进程 */

                if (execv(fmap->path, fmap->argv)) {
                    printf("%s: %s: execv failed!\n", SRV_NAME, __func__);
                    exit(-1);
                }
            }
        }
    }
    return 0;
}
