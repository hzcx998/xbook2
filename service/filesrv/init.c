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

#include "fatfs.h"
#include "filesrv.h"

#define DEBUG_LOCAL 0

/* 最多可以挂载的驱动器 */
#define FILESRV_DRV_NR   10

#define SECTORS_PER_BLOCK   256

/* 创建文件系统状态：0，不创建文件系统，1创建文件系统 */
#define MKFS_STATE  0

/* 原始磁盘，不在上面挂载文件系统 */
#define RAW_DISK    "ide0"

FATFS fatfs_info_table[FILESRV_DRV_NR];           /* Filesystem object */

/* 文件系统驱动器：需要导出给fatfs的diskio.c使用 */
disk_drive_t disk_drives[FILESRV_DRV_NR];

/**
 * filesrv_probe_device - 探测设备
 * 
 */
int filesrv_probe_device()
{
    int idx = 0;
    /* 磁盘设备 */
    devent_t *p = NULL;
    devent_t devent;
    do {
        if (dev_scan(p, DEVICE_TYPE_DISK, &devent))
            break;
#if DEBUG_LOCAL == 1
        printf("%s: %s: probe device %s\n", SRV_NAME, __func__, devent.de_name);
#endif      
        if (strcmp(devent.de_name, RAW_DISK)) {
            disk_drives[idx].devent = devent;
            disk_drives[idx].seq[0] = idx + '0';
            disk_drives[idx].seq[1] = ':';
            disk_drives[idx].seq[2] = 0;
            disk_drives[idx].handle = -1;

            idx++;
            if (idx >= FILESRV_DRV_NR)
                break;
        }
        
        p = &devent;
    } while (1);

    /* 虚拟磁盘设备 */
    p = NULL;
    do {
        if (dev_scan(p, DEVICE_TYPE_VIRTUAL_DISK, &devent))
            break;
#if DEBUG_LOCAL == 1
        printf("%s: %s: probe device %s\n", SRV_NAME, __func__, devent.de_name);
#endif        
        disk_drives[idx].devent = devent;
        disk_drives[idx].seq[0] = idx + '0';
        disk_drives[idx].seq[1] = ':';
        disk_drives[idx].seq[2] = 0;
        disk_drives[idx].handle = -1;

        idx++;
        if (idx >= FILESRV_DRV_NR)
            break;
        p = &devent;
    } while (1);

    int i;
    for (i = 0; i < idx; i++) {
        printf("%s: disk seq: %s -> %s\n", SRV_NAME, disk_drives[i].seq,
            disk_drives[i].devent.de_name);
    }

    return 0;
}

int filesrv_init()
{
    FRESULT res;        /* API result code */
    BYTE work[FF_MAX_SS]; /* Work area (larger is better for processing time) */

    disk_drive_t *dd;
    /* 探测磁盘 */
    filesrv_probe_device();
    int i;
    for (i = 0; i < FILESRV_DRV_NR; i++) {
        dd = &disk_drives[i];
        if (dd->seq[0] != 0) {
#if DEBUG_LOCAL == 1
            printf("%s: %s: make file system on drive %s start.\n", SRV_NAME, __func__, dd->seq);
#endif             
            if (dd->devent.de_type == DEVICE_TYPE_DISK) {
                /* 检测磁盘是否有引导标志 */
                char buf[512];
                memset(buf, 0, 512);
                int diskres = res_open(dd->devent.de_name, RES_DEV, 0);
                if (diskres < 0) {
                    printf("%s: open disk %s failed!\n", SRV_NAME, dd->devent.de_name);
                    return -1;
                }
                res_read(diskres, 0, buf, 512);
                res_close(diskres);
                //printf("%x %x\n", buf[510], buf[511]);
                if (buf[510] != 0x55 && buf[511] != 0xaa) {
                    printf("%s: no fs on %s, create new one.\n", SRV_NAME, dd->devent.de_name);
                    
                    /* 在磁盘上创建文件系统 */
                    res = f_mkfs(dd->seq, 0, work, sizeof(work));
                    if (res != FR_OK) {
                        printf("%s: make fs on drive %s failed with resoult code %d.\n", SRV_NAME, res);
                        return res;
                    }
                }
            } else {    /* 虚拟磁盘每次都需要创建文件系统 */
                /* 在磁盘上创建文件系统 */
                res = f_mkfs(dd->seq, 0, work, sizeof(work));
                if (res != FR_OK) {
                    printf("%s: %s: make fs on drive %s failed with resoult code %d.\n", SRV_NAME, __func__, res);
                    return res;
                }
            }
            //printf("%s: mount drive:%s\n", SRV_NAME, dd->seq);
            /* 挂载到内存中 */
            res = f_mount(&fatfs_info_table[i], dd->seq, 1);
            if (res != FR_OK) {
                printf("%s: %s: mount fs on drive %s failed with resoult code %d.\n", SRV_NAME, __func__, res);
                return res;
            }
            
#if DEBUG_LOCAL == 1
            f_chdrive(dd->seq);
            memset(cwdbuf, 0, 32);

            f_getcwd(cwdbuf, 32);
            //printf("%s: %s: list path %s\n", SRV_NAME, __func__, cwdbuf);
            memset(pathbuf, 0, 256);
            strcpy(pathbuf, dd->seq);

            fatfs_scan_files(pathbuf);
#endif        
        }
    }
#if DEBUG_LOCAL == 1
    /* 切换回第一个驱动器 */
    dd = &disk_drives[0];
    f_chdrive(dd->seq);
#endif
    //printf("%s: init done.\n", SRV_NAME);
    return 0;
}


int filesrv_create_file(char *path, size_t size, char *diskname, long off)
{
    FIL fil;            /* File object */
    FRESULT res;        /* API result code */
    UINT bw;            /* Bytes written */

    long sectors = size / SECTOR_SIZE + 1;
    unsigned char *buf = malloc(sectors * SECTOR_SIZE);
    if (buf == NULL) {
        printf("%s: %s: malloc failed!\n", SRV_NAME, __func__);
        return -1;
    }
    memset(buf, 0, size);
    //printf("%s: %s: malloc ok.\n", SRV_NAME, __func__);
    
    res = f_open(&fil, path, FA_CREATE_ALWAYS | FA_WRITE | FA_READ);
    if (res != FR_OK) {
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
    
    f_write(&fil, buf, size, &bw);
    if (bw != size) {
        printf("%s: %s: f_write failed!\n", SRV_NAME, __func__);
    }
    f_close(&fil);
    free(buf);
    return 0;
close_file:
    f_close(&fil);
    
free_buf:
    free(buf);

    return -1;
}

#define PATH_GUISRV "0:/guisrv"
#define PATH_NETSRV "0:/netsrv.xsv"

/* 文件映射 */
struct file_map {
    char *path;     /* 路径 */
    size_t size;    /* 实际大小 */
    off_t off;      /* 偏移 */
    char execute;   /* 是否需要执行 */
};

#define ARRAY_SIZE(array) (sizeof(array) / sizeof((array)[0]))

struct file_map file_map_table[] = {
    {PATH_GUISRV, 100 * 512, 800, 1},
    {PATH_NETSRV, 400 * 512, 1500, 1},
    {"/login", 100 * 512, 4000, 0},
    {"/bosh", 100 * 512, 4100, 0},
    {"/test", 100 * 512, 4300, 0},
};

/*
[dir] [file] [size] [off]
<bin/abc, hello, 1023, 100>
<srv, guisrv.xsv, 1023, 200>
*/
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
    int pid;
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
                if (execv(fmap->path, NULL)) {
                    printf("%s: %s: execv failed!\n", SRV_NAME, __func__);
                    exit(-1);
                }
            }
        }
    }
    return 0;
}
