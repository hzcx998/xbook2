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
#include <ff.h>

#include <stdio.h>
#include <pthread.h>

#define TTY_NAME    "tty0"

#define DISK_NAME   "ide0"
//#define DISK_NAME   "vfloppy"

/* login arg */
#define BIN_OFFSET      300
#define BIN_SECTORS     300
#define BIN_SIZE        (BIN_SECTORS*512)
#define BIN_NAME        "login"

void build_fatfs();

FATFS fatfs_info;           /* Filesystem object */

FRESULT fatfs_scan_files (
    char* path        /* Start node to be scanned (***also used as work area***) */
);

/**
 * initsrv - 初始化服务
 * 
 * 启动其它服务进程，以及等待所有子进程。
 * 启动的服务可以通过配置服务列表来选择。
 * 
 * 
 * 
 */

int main(int argc, char *argv[])
{
    res_open(TTY_NAME, RES_DEV, 0);
    res_open(TTY_NAME, RES_DEV, 0);
    res_open(TTY_NAME, RES_DEV, 0);
    res_ioctl(RES_STDINNO, TTYIO_CLEAR, 0);
    printf("initsrv: start.\n");

    int pid = fork();
    if (pid < 0) {
        printf("initsrv: fork error! abort service.\n");
        return -1;
    }
    if (pid > 0) {
        while (1) {
            int status = 0;
            int _pid;
            _pid = waitpid(-1, &status, 0);    /* wait any child exit */
            if (_pid > 1) {
                printf("initsrv: process[%d] exit with status %x.\n", _pid, status);
            }
        }
    } else {
        /* filesrv */
        printf("filesrv: start.\n");
        build_fatfs();

        
        unsigned char *buf = malloc(BIN_SIZE);
        if (buf == NULL) {
            printf("malloc failed!\n");
            exit(-1);
        }
        memset(buf, 0, BIN_SIZE);
       // printf("init-child: alloc data at %x for 40 kb.\n", buf);
        
#if 0
        /* open disk */
        int ide0 = res_open(DISK_NAME, RES_DEV, 0);
        if (ide0 < 0) {
            printf("init-child: open disk '%s' %d failed! exit now.", DISK_NAME, ide0);
            return -1;
        }
        
        /* read disk sector for file: offset=200, sectors=50 */
        if (res_read(ide0, BIN_OFFSET, buf, BIN_SIZE / 2) < 0) {
            printf("init-child: read disk sectors failed! exit now\n");
            return -1;
        }
        if (res_read(ide0, BIN_OFFSET + 150, buf + BIN_SIZE / 2, BIN_SIZE / 2) < 0) {
            printf("init-child: read disk sectors failed! exit now\n");
            return -1;
        }
        printf("filesrv: read done.\n");
#endif
#if 1   /* read file data */
        
        FIL fil;            /* File object */
        FRESULT res;        /* API result code */
        UINT br;            /* Bytes written */

        res = f_open(&fil, "hd1:/netsrv.xsv", FA_OPEN_EXISTING | FA_READ);
        if (res != FR_OK) {
            printf("open file failed!\n");
            exit(res);
        }
        printf("read ...\n");
        res = f_read(&fil, buf, BIN_SIZE, &br);
        if (res != FR_OK) {
            printf("read failed! %d\n", res);
            exit(res);
        }
        printf("read done!\n");
        
        if (br != BIN_SIZE) {
            printf("read file failed! read %d\n", br);
            exit(br);
        }
        f_close(&fil);
#endif
        //printf("init-child: load data success.\n");
        
        kfile_t file = {buf, BIN_SIZE};
        /*int i;
        for (i = 0; i < 32; i++) {
            printf("%x ", buf[i]);
        }*/
        //printf("\ninit-child: free resource.\n");
        //res_close(ide0); /* close ide0 */

        char *_argv[4] = {BIN_NAME, "xbook", "1234", 0};
        exit(execfile(BIN_NAME, &file, _argv));
    }
    return 0;
}

/* 文件系统驱动器 */
static char *fs_drives[] = {
    "hd1:", 
    0,
};

void build_fatfs()
{
    
    FIL fil;            /* File object */
    FRESULT res;        /* API result code */
    UINT bw, br;            /* Bytes written */
    BYTE work[FF_MAX_SS]; /* Work area (larger is better for processing time) */

    /* 打印路径的缓冲区 */
    char pathbuf[256];
    char cwdbuf[32];
    char **drv_name = &fs_drives[0];
    while (*drv_name != NULL)
    {
        printf("filesrv: make file system on drive %s start.\n", *drv_name);
#if 0   /* mkfs */
        /* 在磁盘上创建文件系统 */
        /*res = f_mkfs(*drv_name, 0, work, sizeof(work));
        if (res != FR_OK) {
            printf("filesrv: make fs on drive %s failed with resoult code %d.\n", res);
            exit(res);
        }*/
#endif        
        /* 挂载到内存中 */
        f_mount(&fatfs_info, *drv_name, 0);
        f_chdrive(*drv_name);
        memset(cwdbuf, 0, 32);
        f_getcwd(cwdbuf, 32);
        printf("filesrv: list drive %s : %s\n", *drv_name, cwdbuf);
#if 0
        res = f_open(&fil, "hd1:test.txt", FA_CREATE_NEW | FA_WRITE | FA_READ);
        if (res != FR_OK) {
            printf("open file failed!\n");
            exit(res);
        }

        f_write(&res, "hello", 5, &bw);

        f_close(&fil);
#endif
        memset(pathbuf, 0, 256);
        strcpy(pathbuf, "hd1:/");
        fatfs_scan_files(pathbuf);
        
        //f_mount(0, *drv_name, 0);
        drv_name++;
    }
    drv_name = &fs_drives[0];

    /* 切换回第一个驱动器 */
    f_chdrive(*drv_name);
    printf("filesrv: init done.\n");

#if 1   /* write file */
    
    unsigned char *buf = malloc(BIN_SIZE);
    if (buf == NULL) {
        printf("malloc failed!\n");
        exit(-1);
    }
    memset(buf, 0, BIN_SIZE);

    res = f_open(&fil, "hd1:/netsrv.xsv", FA_CREATE_ALWAYS | FA_WRITE | FA_READ);
    if (res != FR_OK) {
        printf("filesrv: open file failed!\n");
        exit(res);
    }
    printf("filesrv: open done.\n");

    // printf("init-child: alloc data at %x for 40 kb.\n", buf);
    printf("filesrv: open done.\n");

    /* open disk */
    int ide0 = res_open(DISK_NAME, RES_DEV, 0);
    if (ide0 < 0) {
        printf("filesrv: open disk '%s' %d failed! exit now.", DISK_NAME, ide0);
        return;
    }
    /* read disk sector for file: offset=200, sectors=50 */
    /*if (res_read(ide0, BIN_OFFSET, buf, BIN_SIZE) < 0) {
        printf("filesrv: read disk sectors failed! exit now\n");
        return;
    }*/
    /* read disk sector for file: offset=200, sectors=50 */
    if (res_read(ide0, BIN_OFFSET, buf, BIN_SIZE / 2) < 0) {
        printf("init-child: read disk sectors failed! exit now\n");
        return -1;
    }
    if (res_read(ide0, BIN_OFFSET + 150, buf + BIN_SIZE / 2, BIN_SIZE / 2) < 0) {
        printf("init-child: read disk sectors failed! exit now\n");
        return -1;
    }
    printf("filesrv: open disk done.\n");

    f_write(&fil, buf, BIN_SIZE, &bw);
    if (bw != BIN_SIZE) {
        printf("filesrv: write file failed!\n");
        return;
    }
    printf("filesrv: write file done.\n");

    f_close(&fil);

    free(buf);
#endif
    printf("filesrv: write done.\n");

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

void fatfs_test()
{
    printf("in fat fs test...\n");

    FATFS fs;           /* Filesystem object */
    FIL fil;            /* File object */
    FRESULT res;        /* API result code */
    UINT bw, br;            /* Bytes written */
    BYTE work[FF_MAX_SS]; /* Work area (larger is better for processing time) */

    res = f_mkfs("hd1:", 0, work, sizeof(work));
    if (res != FR_OK) {
        printf("make fs on ram failed! %d \n", res);
        exit(res);
    }
    f_mount(&fs, "hd1:", 0);

    res = f_open(&fil, "hd1:test.txt", FA_CREATE_NEW | FA_WRITE | FA_READ);
    if (res != FR_OK) {
        printf("open file failed!\n");
        exit(res);
    }

    res = f_write(&fil, "hello, file!\n", 13, &bw);
    if (res != FR_OK) {
        printf("open write failed! %d\n", res);
        exit(res);
    }
    if (bw != 13) {
        printf("write file failed!\n");
        exit(bw);
    }
    printf("write file %d!\n", bw);

    f_lseek(&fil, 0);

    char fbuf[32];
    res = f_read(&fil, fbuf, bw, &br);
    if (res != FR_OK) {
        printf("open read failed! %d\n", res);
        exit(res);
    }

    printf("read bytes:%d -> %s\n", br, fbuf);

    f_printf(&fil, "hello, f_printf %d %x %s", 123, 0x123, "123");

    f_lseek(&fil, 13);
    
    res = f_read(&fil, fbuf, 32, &br);
    if (res != FR_OK) {
        printf("open read failed! %d\n", res);
        exit(res);
    }
    printf("read bytes:%d -> %s\n", br, fbuf);

    f_close(&fil);

    f_chdrive("hd1:");

    memset(fbuf, 0, 32);
    res = f_getcwd(fbuf, 32);
    if (res != FR_OK) {
        printf("getcwd failed! %d\n", res);
        exit(res);        
    } 
    printf("cwd/%s\n", fbuf);

    res = f_mkdir("/bin");
    if (res != FR_OK) {
        printf("f_mkdir %s failed! %d\n", "/bin", res);
        exit(res);
    }
    f_mkdir("/usr");
    if (res != FR_OK) {
        printf("f_mkdir %s failed! %d\n", "/usr", res);
        exit(res);
    }
    f_mkdir("/bin/supper");
    if (res != FR_OK) {
        printf("f_mkdir %s failed! %d\n", "/bin/supper", res);
        exit(res);
    }
    char pathbuf[256];
    strcpy(pathbuf, "/");
    //scan_files(pathbuf);

    f_mount(NULL, "hd1:", 0);
    printf("test fatfs done!\n");
    
    /* mount drive */
    f_mount(&fs, "hd1:", 0);

    /* load disk to file system */
    /* open disk */
    int ide0 = res_open(DISK_NAME, RES_DEV, 0);
    if (ide0 < 0) {
        printf("init-child: open disk '%s' %d failed! exit now.", DISK_NAME, ide0);
        return;
    }
    unsigned char *buf = malloc(BIN_SIZE);
    if (buf == NULL) {
        printf("malloc failed!\n");
        exit(-1);
    }
    memset(buf, 0, BIN_SIZE);
    // printf("init-child: alloc data at %x for 40 kb.\n", buf);
    
    /* read disk sector for file: offset=200, sectors=50 */
    if (res_read(ide0, BIN_OFFSET, buf, BIN_SIZE) < 0) {
        printf("init-child: read disk sectors failed! exit now\n");
        return;
    }

    memset(fbuf, 0, 32);
    res = f_getcwd(fbuf, 32);
    if (res != FR_OK) {
        printf("getcwd failed! %d\n", res);
        exit(res);        
    } 
    printf("cwd/%s\n", fbuf);

    res = f_open(&fil, "hd1:login", FA_CREATE_NEW | FA_WRITE | FA_READ);
    if (res != FR_OK) {
        printf("open file failed!\n");
        exit(res);
    }

    res = f_write(&fil, buf, BIN_SIZE, &bw);
    if (res != FR_OK) {
        printf("open write failed! %d\n", res);
        exit(res);
    }
    if (bw != BIN_SIZE) {
        printf("write file failed!\n");
        exit(bw);
    }
    printf("write file %d!\n", bw);
    f_close(&fil);
    f_mount(0, "hd1:", 0);
    free(fbuf);
    return;
    while (1)
    {
        /* code */
    }
}