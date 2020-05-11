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
#include <ff.h>

#include <stdio.h>
#include <pthread.h>

#include "filesrv.h"

/* 创建文件系统状态：0，不创建文件系统，1创建文件系统 */
#define MKFS_STATE  0

#define DISK_NAME   "ide0"

/* 最多可以挂载的驱动器 */
#define FILESRV_DRV_NR   10

#define SECTORS_PER_BLOCK   256

FATFS fatfs_info_table[FILESRV_DRV_NR];           /* Filesystem object */

FRESULT fatfs_scan_files (
    char* path        /* Start node to be scanned (***also used as work area***) */
);

int filesrv_init();
int filesrv_create_files();
int filesrv_execute();

int execv(const char *path, const char *argv[]);


/**
 * filesrv - 文件服务
 */
int main(int argc, char *argv[])
{
    printf("%s: started.\n", SRV_NAME);

    /* 初始化文件系统 */
    if (filesrv_init()) { /* 创建文件系统 */
        printf("%s: init filesystem failed, service stopped!\n", SRV_NAME);
        return -1;
    }

    if (filesrv_file_table_init()) {
        printf("%s: init file table failed, service stopped!\n", SRV_NAME);
        return -1;
    }
    
    if (filesrv_init_interface()) {
        printf("%s: init interface failed, service stopped!\n", SRV_NAME);
        return -1;
    }

    /* 构建文件 */
    if (filesrv_create_files()) {
        printf("%s: create files failed, service stopped!\n", SRV_NAME);
        return -1;
    }
    
    /* 启动其它程序 */
    if (filesrv_execute()) {
        printf("%s: execute failed, service stopped!\n", SRV_NAME);
        return -1;
    }
    
    if (srvcall_bind(SRV_FS))  {
        printf("%s: bind srvcall failed, service stopped!\n", SRV_NAME);
        return -1;
    }
    FIL file0;
    FRESULT fres;
    int seq = 0;
    char buf0[32];
    char *str = "filesrv->read."; 
    srvarg_t srvarg;
    int callnum;
    while (1)
    {
        /* 接收消息 */
        /* 处理消息 */
        /* 反馈消息 */
        memset(&srvarg, 0, sizeof(srvarg_t));
        if (srvcall_listen(SRV_FS, &srvarg)) {
            continue;
        }
        callnum = srvarg.data[0];
        printf("%s: srvcall num %d\n", SRV_NAME, callnum);

        if (callnum >= 0 && callnum < FILESRV_CALL_NR) {
            filesrv_call_table[callnum](&srvarg);
        }
        #if 0
        switch (srvarg.data[0])
        {
        case FILESRV_OPEN:
            
            /* 现在已经获取了完整的数据 */
            printf("%s: data1 %x\n", SRV_NAME, srvarg.data[1]);
            printf("path: %s\n", (char *) srvarg.data[1]);
            printf("%s: data2 %x\n", SRV_NAME, srvarg.data[2]);
            
            fres = f_open(&file0, (char *) srvarg.data[1], FA_CREATE_ALWAYS | FA_READ | FA_WRITE);
            if (fres != FR_OK) {
                srvarg.retval = 0;  /* null */
            } else {
                srvarg.retval = (unsigned long) &file0;
            }
            break;
        case FILESRV_CLOSE:
            printf("%s: srvcall [close]\n", SRV_NAME);
            printf("%s: data1 %x\n", SRV_NAME, srvarg.data[1]);
            fres = f_close((FIL *)srvarg.data[1]);
            if (fres != FR_OK) {
                srvarg.retval = -1;  /* null */
            } else {
                srvarg.retval = 0;
            }
            break;
        case FILESRV_READ:
            printf("%s: srvcall [read]\n", SRV_NAME);
            printf("%s: data1 %x\n", SRV_NAME, srvarg.data[1]);
            /* 检测参数 */
            if (!srvcall_check(&srvarg)) {
                srvarg.data[2] = (unsigned long) &buf0[0];
                if (srvcall_fetch(SRV_FS, &srvarg)) {
                    break;
                }
            }
            srvarg.data[1] = (unsigned long) str;
            srvarg.size[1] = strlen(str);
            
            /* 现在已经获取了完整的数据 */
            printf("%s: data1 %x\n", SRV_NAME, srvarg.data[1]);
            printf("%s: data2 %x\n", SRV_NAME, srvarg.data[2]);
            printf("str: %s\n", (char *) srvarg.data[2]);
            srvarg.retval = 123;
            break;
        default:
            break;
        }
        #endif
        printf("%s: srvcall listen ok! %d\n", SRV_NAME, seq);
        
        srvcall_ack(SRV_FS, &srvarg);
        seq++;
            
    }
    return 0;
}

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
        printf("%s: %s: probe device %s\n", SRV_NAME, __func__, devent.de_name);
        
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

    /* 虚拟磁盘设备 */
    p = NULL;
    do {
        if (dev_scan(p, DEVICE_TYPE_VIRTUAL_DISK, &devent))
            break;
        printf("%s: %s: probe device %s\n", SRV_NAME, __func__, devent.de_name);
        
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
    return 0;
}

int filesrv_init()
{
    FRESULT res;        /* API result code */
    BYTE work[FF_MAX_SS]; /* Work area (larger is better for processing time) */

    /* 打印路径的缓冲区 */
    char pathbuf[256];
    char cwdbuf[32];
    disk_drive_t *dd;
    /* 探测磁盘 */
    filesrv_probe_device();
    int i;
    for (i = 0; i < FILESRV_DRV_NR; i++) {
        dd = &disk_drives[i];
        if (dd->seq[0] != 0) {
            printf("%s: %s: make file system on drive %s start.\n", SRV_NAME, __func__, dd->seq);
             
            if (dd->devent.de_type == DEVICE_TYPE_DISK) {
#if MKFS_STATE == 1   /* mkfs */
                /* 在磁盘上创建文件系统 */
                res = f_mkfs(dd->seq, 0, work, sizeof(work));
                if (res != FR_OK) {
                    printf("%s: make fs on drive %s failed with resoult code %d.\n", SRV_NAME, res);
                    return res;
                }
#endif
            } else {    /* 虚拟磁盘每次都需要创建文件系统 */
                /* 在磁盘上创建文件系统 */
                res = f_mkfs(dd->seq, 0, work, sizeof(work));
                if (res != FR_OK) {
                    printf("%s: %s: make fs on drive %s failed with resoult code %d.\n", SRV_NAME, __func__, res);
                    return res;
                }
            }

            /* 挂载到内存中 */
            res = f_mount(&fatfs_info_table[i], dd->seq, 0);
            if (res != FR_OK) {
                printf("%s: %s: mount fs on drive %s failed with resoult code %d.\n", SRV_NAME, __func__, res);
                return res;
            }
            f_chdrive(dd->seq);
            memset(cwdbuf, 0, 32);
            f_getcwd(cwdbuf, 32);
            printf("%s: %s: list path %s\n", SRV_NAME, __func__, cwdbuf);
            memset(pathbuf, 0, 256);
            strcpy(pathbuf, dd->seq);
            fatfs_scan_files(pathbuf);
        }
    }
    /* 切换回第一个驱动器 */
    dd = &disk_drives[0];
    f_chdrive(dd->seq);
    printf("%s: init done.\n", SRV_NAME);
    return 0;
}

/**
 * filesrv_exit - 正常退出服务
 * 
 */
int filesrv_exit()
{
    
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

#define PATH_NETSRV "1:/netsrv.xsv"


int filesrv_create_files()
{
    if (filesrv_create_file(PATH_NETSRV, 200 * 1024, "ide0", 800)) {
        return -1;
    }
    return 0;
}

int filesrv_execute()
{
    int pid = fork();
    if (pid < 0) {
        printf("%s: %s: fork failed!\n", SRV_NAME, __func__);
        return -1;
    }
    if (!pid) { /* 子进程执行新进程 */
        if (execv(PATH_NETSRV, NULL)) {
            printf("%s: %s: execv failed!\n", SRV_NAME, __func__);
            exit(-1);
        }    
    }
    return 0;
}

/**
 * execv - 执行程序，替换当前进程镜像
 * 
 */
int execv(const char *path, const char *argv[])
{
    /* netsrv */
    FIL fil;
    FRESULT fres;
    int bw;
    fres = f_open(&fil, path, FA_OPEN_EXISTING | FA_READ);

    if (fres != FR_OK) {
        printf("%s: %s: f_open failed!\n", SRV_NAME, __func__);
        return -1;
    }

    unsigned char *buff = malloc(f_size(&fil));
    if (buff == NULL) {
        f_close(&fil);
        return -1;
    }

    fres = f_read(&fil, buff, (UINT)f_size(&fil), (UINT *)&bw);    
    if (fres != FR_OK || bw != f_size(&fil)) {
        f_close(&fil);
        free(buff);
        return -1;
    }
    f_close(&fil);
    
    kfile_t file;
    file.file = buff;
    file.size = bw;

    char *name = strrchr(path, '/');
    name++;
    if (name[0] == 0) { /* 后面没有名字 */
        name = (char *) path;
    }
    //printf("%s: %s: process name %s\n", SRV_NAME, __func__, name);
    execfile(name, &file, (char **) argv);
    return -1;
}

int execl(const char *path, const char *arg, ...)
{
	va_list parg = (va_list)(&arg);
	const char **p = (const char **) parg;
	return execv(path, p);
}

FRESULT fatfs_scan_files (
    char* path        /* Start node to be scanned (***also used as work area***) */
)
{
    FRESULT res;
    DIR dir;
    UINT i;
    static FILINFO fno;

    res = f_opendir(&dir, path);                       /* Open the directory */
    if (res == FR_OK) {
        for (;;) {
            res = f_readdir(&dir, &fno);                   /* Read a directory item */
            if (res != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
            if (fno.fattrib & AM_DIR) {                    /* It is a directory */
                printf("%s/%s\n", path, fno.fname);
                i = strlen(path);
                sprintf(&path[i], "/%s", fno.fname);
                res = fatfs_scan_files(path);                    /* Enter the directory */
                if (res != FR_OK) break;
                path[i] = 0;
            } else {                                       /* It is a file. */
                printf("%s/%s\n", path, fno.fname);
            }
        }
        f_closedir(&dir);
    }

    return res;
}
