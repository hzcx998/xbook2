#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "driver.h"

/* 10MB 镜像文件 */
#define SECTOR_SIZE     512 

/*
#define DISK_SECTORS    (20480*10)   
#define DISK_SIZE       (DISK_SECTORS * SECTOR_SIZE)    
*/


// #define DEBUG_FATFS_DRIVER

FILE *drv_file;
void *drv_buf;

char disk_path[256];
char disk_name[32];

int open_ref;

int mkfs_flags;
extern int rom_disk;   /* rom is always new */

/* 磁盘大小 */
int disk_size;
/* 磁盘扇区数 */
int disk_sectors;

int drv_init(char *path, int disk_sz)
{
    open_ref = 0;
    mkfs_flags = 0;
    drv_buf = NULL;
    
    memset(disk_path, 0, 256);
    strcpy(disk_path, path);

    if (rom_disk) { /* new one */
#ifdef DEBUG_FATFS_DRIVER
        printf("fatfs: a rom disk.\n");
#endif
        drv_file = fopen(disk_path, "wb+");    
        if (drv_file == NULL) {
            printf("fatfs: open disk %s failed!\n", disk_path);
            return -1;
        }
    } else {    /* can open old disk */
#ifdef DEBUG_FATFS_DRIVER
        printf("fatfs: a traditional disk.\n");
#endif
        drv_file = fopen(disk_path, "rb+");    
        if (drv_file == NULL) { /* 文件存在则继续，不存在则失败 */
    #ifdef DEBUG_FATFS_DRIVER
            printf("fatfs: create a new disk.\n");
    #endif
            if (disk_sz == 0) { /* 磁盘为0表示不创建新的 */
                printf("fatfs: disk %s not exist, please create the disk first!\n", path);
                return -1;
            }
            drv_file = fopen(disk_path, "wb+");    
            if (drv_file == NULL) {
                printf("fatfs: open disk %s failed!\n", disk_path);
                return -1;
            }
        }
    }
    
    fseek(drv_file, 0, SEEK_END);
    int filesz = ftell(drv_file);
    if (filesz <= 0) { /* 创建一个新的镜像文件 */
#ifdef DEBUG_FATFS_DRIVER        
        printf("fatfs: build a new disk.\n");
#endif
        /* 文件大小是参数中的大小 */
        disk_size = disk_sz;
        disk_sectors = disk_size / SECTOR_SIZE;

        /* 文件存在，且指针在最前面 */
        drv_buf = malloc(disk_size);
        if (drv_buf == NULL) {
            printf("fatfs: malloc for disk buf failed!\n");
            fclose(drv_file);
            return -1;
        }
        memset(drv_buf, 0, disk_size);

        //mkfs_flags = 1;    /* 一块新的磁盘，需要创建文件系统 */
        if (fwrite(drv_buf, disk_size / 10, 10,  drv_file) < 0) {
            printf("fatfs: build a new disk failed!\n");
            fclose(drv_file);
            return -1;
        }

        fflush(drv_file);
    } else {    /* 已经存在的话，那么文件大小就是磁盘文件大小 */
        
        disk_size = filesz;
        disk_sectors = filesz / SECTOR_SIZE;
    }
#ifdef DEBUG_FATFS_DRIVER    
    printf("fatfs: disk size=%d setctors=%d.\n", disk_size, disk_sectors);
#endif
    if (drv_buf == NULL) {
        /* 文件存在，且指针在最前面 */
        drv_buf = malloc(disk_size);
        if (drv_buf == NULL) {
            printf("fatfs: malloc for disk buf failed!\n");
            fclose(drv_file);
            return -1;
        }
        memset(drv_buf, 0, disk_size);
    }
    
    fseek(drv_file, 0, SEEK_END);
    filesz = ftell(drv_file);
#ifdef DEBUG_FATFS_DRIVER
    printf("fatfs: disk size is %d, %d MB\n", filesz, filesz / (1024*1024));
#endif
    rewind(drv_file);

    /* 读取0扇区，检测是否需要创建文件系统 */
    unsigned char bootbuf[SECTOR_SIZE] = {0};
    if (fread(bootbuf, SECTOR_SIZE, 1, drv_file) < 0) {
        printf("fatfs: read boot sector failed!\n");
        fclose(drv_file);
        free(drv_buf);
        return -1;
    }
    /* 如果没有引导，说明需要创建文件系统 */
    if (!(bootbuf[510] == 0x55 && bootbuf[511] == 0xAA)) {
        mkfs_flags = 1;
#ifdef DEBUG_FATFS_DRIVER   
        printf("fatfs: need make fs on disk.\n");
#endif
    }
    rewind(drv_file);

    return 0;
}

int drv_open()
{
    //printf("fatfs: call open driver!\n");
    
    if (open_ref > 0) {
        open_ref++;
        return 0;
    }
#ifdef DEBUG_FATFS_DRIVER   
    printf("fatfs: do open driver.\n");
#endif    
    /* 加载磁盘数据到内存 */
    rewind(drv_file);
    if (fread(drv_buf, disk_size / 10, 10,  drv_file) <= 0) {
        printf("fatfs: load disk to ram failed!\n");
        return -1;
    }
    rewind(drv_file);

    open_ref++;
    return 0;
}

int drv_close()
{
    //printf("fatfs: call close driver!\n");
    open_ref--;
    if (open_ref > 0) {
        return 0;
    }
#ifdef DEBUG_FATFS_DRIVER   
    printf("fatfs: do close driver.\n");
#endif
    rewind(drv_file);
    fwrite(drv_buf, disk_size / 10, 10, drv_file);
    free(drv_buf);
    return fclose(drv_file); 
}

int drv_read(unsigned int off, void *buffer, size_t count)
{
    if (off + count > disk_sectors)
        return -1;
    unsigned char *pos;
    pos = (unsigned char *) drv_buf + off * SECTOR_SIZE;
    memcpy(buffer, pos, count * SECTOR_SIZE);
    return count;
}

int drv_write(unsigned int off, void *buffer, size_t count)
{
    if (off + count > disk_sectors)
        return -1;
    unsigned char *pos;
    pos = (unsigned char *) drv_buf + off * SECTOR_SIZE;
    memcpy(pos, buffer, count * SECTOR_SIZE);
    return count;
}

int drv_ioctl(int cmd, unsigned long arg)
{
    switch(cmd)
    {
    case DISKIO_SYNC:
        rewind(drv_file);
        fwrite(drv_buf, disk_size / 10, 10, drv_file);
        fflush(drv_file);
        rewind(drv_file);
        
        break;
    case DISKIO_GETSSIZE:
        *(unsigned int *)arg = SECTOR_SIZE;
        break;     
    case DISKIO_GETSIZE:
        *(unsigned int *)arg = disk_sectors;
        break;
    default:
        return -1;
    }
    return 0;
}
