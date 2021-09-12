#include <stdio.h>
#include <string.h>
#include "driver.h"
#include <stdlib.h>

#ifdef WIN32
#include <io.h>
#else
#include <sys/dir.h>
#include <dirent.h>
#include <unistd.h>
#endif

#include "ff.h"

// #define DEBUG_FATFS_MAIN

/* 磁盘驱动，把镜像文件模拟成一个磁盘设备 */

/* 镜像目录 */
//#define IMAGE_FILE "D:/Developmemts/github/xbook2/develop/image/hd.img"
#define IMAGE_FILE "../../develop/image/d.img"

/* 主机rom目录 */
//#define ROM_DIR "D:/Developmemts/github/xbook2/tools/rom"
#define ROM_DIR "../develop/rom"

/* 磁盘是否为ROM */
#define ROM_DIS 0

/* 默认磁盘大小 */
#define DISK_SIZE       (10*1024*1024)    

extern int mkfs_flags;

int rom_disk;   /* rom is always new */

FRESULT scan_files (
    char* path        /* Start node to be scanned (***also used as work area***) */
);

int scan_host_files(char *host_path, char *custom_path);

/**
 * 参数：
 * fatfs 磁盘镜像路径 rom目录 磁盘大小（MB）
 * 
 */
int main(int argc, char *argv[]) 
{
#ifdef DEBUG_FATFS_MAIN   
    printf("==== FATFS MANAGMENT ====\n");
    printf("args:\n");
    int i;
    for (i = 0; i < argc; i++) {
        printf("argv[%d]=%s\n", i, argv[i]);
    }
#endif    
    if (argc > 4) {
        printf("fatfs: args error!\n");
        printf("usage: fatfs [image file] [rom dir] disksz\n");
        return -1;
    }
    char *image_file;
    char *rom_dir;
    int disk_sz;
    if (argc >= 3) {
        image_file = argv[1];
        rom_dir = argv[2];
        disk_sz = atoi(argv[3]);    /* 单位是MB */
        if (disk_sz > 0) 
            disk_sz *= (1024*1024); /* 转换成MB */
        
    } else {
        image_file = IMAGE_FILE;
        rom_dir = ROM_DIR;
        disk_sz = DISK_SIZE;
    }
#ifdef DEBUG_FATFS_MAIN   
    printf("fatfs: image file: %s\n", image_file);
    printf("fatfs: rom dir: %s\n", rom_dir);
#endif    
    rom_disk = ROM_DIS;

    int retval = 0;
    /* 初始化驱动 */
    if (drv_init(image_file, disk_sz) < 0) {
        printf("fatfs: init driver failed!\n");
        return -1;
    }

    FATFS fs;           /* Filesystem object */
    FIL fil;            /* File object */
    FRESULT res;        /* API result code */
    UINT bw;            /* Bytes written */
    BYTE work[FF_MAX_SS]; /* Work area (larger is better for processing time) */

    if (mkfs_flags) {
#ifdef DEBUG_FATFS_MAIN           
        printf("fatfs: make a new fs on the disk.\n");
#endif
        //MKFS_PARM opt = {FM_FAT32, 0, 0, 0, 0};
        /* Format the default drive with default parameters */
        res = f_mkfs("", 0, work, sizeof work);
        if (res) {
            printf("fatfs: f_mkfs error with %d\n", res);
            retval = -1;
            goto scroll_close_driver;
        }
    }

    /* Gives a work area to the default drive */
    if (f_mount(&fs, "", 0) != FR_OK) {
        printf("fatfs: mount fs on disk failed!\n");
        retval = -1;
        goto scroll_close_driver;
    }

    /* 打印rom目录内容 */
    #ifdef DEBUG_FATFS_MAIN   
    printf("==== HOST ROM FILES ====\n");
    #endif
    scan_host_files(rom_dir, "0:");

    /* 打印磁盘文件上的文件信息 */
    #ifdef DEBUG_FATFS_MAIN   
    printf("==== FILES ====\n");
    #endif
    char dir_buf[256] = {0};
    strcpy(dir_buf, "0:");
    scan_files(dir_buf);

    /* Unregister work area */
    f_mount(0, "", 0);

    printf("fatfs: write rom files success.\n");

scroll_close_driver:
    /* 关闭驱动 */
    drv_close();
    return retval;
}

FRESULT scan_files (
    char* path        /* Start node to be scanned (***also used as work area***) */
)
{
    FRESULT res;
    FDIR dir;
    UINT i;
    static FILINFO fno;

    res = f_opendir(&dir, path);                       /* Open the directory */
    if (res == FR_OK) {
        for (;;) {
            res = f_readdir(&dir, &fno);                   /* Read a directory item */
            if (res != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
            if (fno.fattrib & AM_FDIR) {                    /* It is a directory */
#ifdef DEBUG_FATFS_MAIN
                printf("%s/%s\n", path, fno.fname);
#endif
                char sub_path[256];
                memset(sub_path, 0, 256);
                strcat(sub_path, path);
                if (sub_path[strlen(path) - 1] != '/') {
                    strcat(sub_path, "/");
                }
                strcat(sub_path,fno.fname);
                
                //i = strlen(path);
                //sprintf(&path[i], "/%s", fno.fname);
                res = scan_files(sub_path);                    /* Enter the directory */
                if (res != FR_OK) break;
                //path[i] = 0;
            } else {                                       /* It is a file. */
                #ifdef DEBUG_FATFS_MAIN    
                printf("%s/%s size: %d\n", path, fno.fname, fno.fsize);
                #endif
            }
        }
        f_closedir(&dir);
    }
    return res;
}

/**
 * copy_file_to_custom - 复制文件到客机
 * 
 */
int copy_file_to_custom(char *host_path, char *custom_path)
{
#ifdef DEBUG_FATFS_MAIN    
    printf("fatfs: copy file from %s to %s\n", host_path, custom_path);
#endif
    FILE *fp = fopen(host_path, "rb+");
    if (fp == NULL) {
        printf("fatfs: error: open host file %s failed!\n", host_path);
        return -1;
    }
    fseek(fp, 0, SEEK_END);
    int filesz = ftell(fp);
#ifdef DEBUG_FATFS_MAIN
    printf("fatfs: host file size:%d\n", filesz);
#endif
    if (filesz <= 0) { /* empty file */
        fclose(fp);
        return -1;
    }

    FIL fil;
    FRESULT fr;
    fr = f_open(&fil, custom_path, FA_CREATE_ALWAYS | FA_WRITE);
    if (fr != FR_OK) {
        printf("fatfs: error: open custom file %s failed!\n",custom_path);
        fclose(fp);
        return -1;
    }

    fseek(fp, 0, SEEK_SET);
    void *buf = malloc(filesz + 1);
    if (buf == NULL) {
        printf("fatfs: error: malloc for file buf about %d bytes failed!\n", filesz);
        f_close(&fil);
        fclose(fp);
        return -1;
    }
    size_t read_bytes;
    memset(buf, 0, filesz + 1);
    read_bytes = fread(buf, 1, filesz,  fp);
#ifdef DEBUG_FATFS_MAIN
    printf("fatfs: read %d bytes.\n", read_bytes);
#endif
    if (read_bytes != filesz || read_bytes <= 0) {
        printf("fatfs: error: read file from %s failed!\n", host_path);
        free(buf);
        f_close(&fil);
        fclose(fp);
        return -1;
    }

    unsigned int fw;
    f_lseek(&fil, 0);
    fr = f_write(&fil, buf, filesz, &fw);
    if (fr != FR_OK || fw != filesz) {
        printf("fatfs: error: write data to custom file %s failed!\n", custom_path);
        free(buf);
        f_close(&fil);
        fclose(fp);
        return -1;
    }
    free(buf);
    f_close(&fil);
    fclose(fp);
    return 0;
}
#ifdef WIN32

/**
 * scan_host_files - 扫描主机上面的目录
 * 
 */
int scan_host_files(char *host_path, char *custom_path)
{
    int rv = 0;
#if 0
    /* 改变当前工作目录 */
    rv = chdir(host_path);
    if (0 != rv) {
        printf("fatfs: scan_host_files: call chdir() failed!\n");
        rv = -1;
        return rv;
    }
#endif
    /* 打印客机目录 */
#ifdef DEBUG_FATFS_MAIN
    printf("[custom path] %s\n", custom_path);
#endif
    struct _finddata_t data;
    long handle;
    char path_buf[1024] = {0};
    memset(path_buf, 0, 1024);
    strcpy(path_buf, host_path);
    strcat(path_buf, "/*");
    if (-1L == (handle = _findfirst(path_buf, &data)))//成功返回唯一的搜索句柄, 出错返回-1
        return rv;

    do {
        char host_buf[1024];
        memset(host_buf, 0, 1024);
        char custom_buf[256] = {0};
        memset(custom_buf, 0, 256);
        if (data.attrib == _A_SUBDIR ) {//目录类型
            if (strcmp(data.name, ".") != 0 && strcmp(data.name, "..") != 0) {
                /* 在客机中创建一个目录 */
                strcat(custom_buf, custom_path);
                if (custom_buf[strlen(custom_path)-1] != '/') {
                    strcat(custom_buf, "/");
                }
                strcat(custom_buf, data.name);
                //sprintf(custom_buf, "%s/%s", custom_path, data.name);
#ifdef DEBUG_FATFS_MAIN
                printf("[custom dir] %s -> %s\n", custom_buf, data.name);
#endif
                //memcpy(custom_path, custom_buf, 256);
                f_mkdir(custom_buf);

                /* 打印主机路径 */
                sprintf(host_buf, "%s/%s", host_path, data.name);
#ifdef DEBUG_FATFS_MAIN
                printf("[host dir] %s -> %s\n", host_buf, data.name);
#endif
                scan_host_files(host_buf, custom_buf);
            }
        } else {//单个文件
            /* 打印主机路径 */
           
            sprintf(host_buf, "%s/%s", host_path, data.name);
#ifdef DEBUG_FATFS_MAIN
            printf("[host file] %s -> %s\n", host_buf, data.name);
#endif
            /* 在客机中创建一个文件 */
            sprintf(custom_buf, "%s/%s", custom_path, data.name);
#ifdef DEBUG_FATFS_MAIN
            printf("[custom file] %s -> %s\n", custom_buf, data.name);
#endif
            copy_file_to_custom(host_buf, custom_buf);
        }
    } while(_findnext( handle, &data ) == 0);     //成功返回0 , 出错返回-1
    
    _findclose( handle );     // 关闭当前句柄

    return rv;

}

#else

/**
 * scan_host_files - 扫描主机上面的目录
 * 
 */
int scan_host_files(char *host_path, char *custom_path)
{
    int rv = 0;
    /* 打印客机目录 */
#ifdef DEBUG_FATFS_MAIN
    printf("[custom path] %s\n", custom_path);
#endif

    DIR *dir;
    struct dirent *de;
    //struct _finddata_t data;
    long handle;
    char path_buf[1024] = {0};
    memset(path_buf, 0, 1024);
    strcpy(path_buf, host_path);

    dir = opendir(host_path);
    if (dir == NULL) {
        return -1;
    }
    
    
    do {
        de = readdir(dir);
        if (de == NULL)
            break;

        char host_buf[1024];
        memset(host_buf, 0, 1024);
        char custom_buf[256] = {0};
        memset(custom_buf, 0, 256);
        if (de->d_type == 4) {//目录类型
            if (strcmp(de->d_name, ".") != 0 && strcmp(de->d_name, "..") != 0) {
                /* 在客机中创建一个目录 */
                strcat(custom_buf, custom_path);
                if (custom_buf[strlen(custom_path)-1] != '/') {
                    strcat(custom_buf, "/");
                }
                strcat(custom_buf, de->d_name);
                //sprintf(custom_buf, "%s/%s", custom_path, de->d_name);
#ifdef DEBUG_FATFS_MAIN
                printf("[custom dir] %s -> %s\n", custom_buf, de->d_name);
#endif
                //memcpy(custom_path, custom_buf, 256);
                f_mkdir(custom_buf);

                /* 打印主机路径 */
                sprintf(host_buf, "%s/%s", host_path, de->d_name);
#ifdef DEBUG_FATFS_MAIN
                printf("[host dir] %s -> %s\n", host_buf, de->d_name);
#endif
                scan_host_files(host_buf, custom_buf);
            }
        } else {//单个文件
            /* 打印主机路径 */
           
            sprintf(host_buf, "%s/%s", host_path, de->d_name);
#ifdef DEBUG_FATFS_MAIN
            printf("[host file] %s -> %s\n", host_buf, de->d_name);
#endif
            /* 在客机中创建一个文件 */
            sprintf(custom_buf, "%s/%s", custom_path, de->d_name);
#ifdef DEBUG_FATFS_MAIN
            printf("[custom file] %s -> %s\n", custom_buf, de->d_name);
#endif
            copy_file_to_custom(host_buf, custom_buf);
        }
    } while(1);     //成功返回0 , 出错返回-1
    
    closedir(dir);     // 关闭当前句柄

    return rv;

}

#endif
