#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "driver.h"
#include <io.h>

/* 10MB 镜像文件 */
#define SECTOR_SIZE     512 
#define DISK_SECTORS    (20480*10)   
#define DISK_SIZE       (DISK_SECTORS * SECTOR_SIZE)    

#define DEBUG_LOCAL 1

FILE *drv_file;
void *drv_buf;

char disk_path[256];
char disk_name[32];

int open_ref;

int is_new_disk;
extern int rom_disk;   /* rom is always new */

int drv_init(char *path)
{
    open_ref = 0;
    is_new_disk = 0;

    memset(disk_path, 0, 256);
    strcpy(disk_path, path);
}

int drv_open()
{
    if (open_ref > 0) {
        open_ref++;
        return 0;
    }
    
    if (rom_disk) { /* new one */
#if DEBUG_LOCAL == 1
        printf("fatfs: a rom disk.\n");
#endif
        drv_file = fopen(disk_path, "w+");    
        if (drv_file == NULL) {
            return -1;
        }
    } else {    /* can open old disk */
#if DEBUG_LOCAL == 1
        printf("fatfs: a traditional disk.\n");
#endif
        drv_file = fopen(disk_path, "r+");    
        if (drv_file == NULL) { /* 文件存在则继续，不存在则失败 */
    #if DEBUG_LOCAL == 1
            printf("fatfs: create a new disk.\n");
    #endif
            drv_file = fopen(disk_path, "w+");    
            if (drv_file == NULL) {
                return -1;
            }
        }
    }
    
    /* 文件存在，且指针在最前面 */
    drv_buf = malloc(DISK_SIZE);
    if (drv_buf == NULL) {
        fclose(drv_file);
        return -1;
    }
    memset(drv_buf, 0, DISK_SIZE);
    fseek(drv_file, 0, SEEK_END);
    int filesz = ftell(drv_file);
    if (filesz <= 0) { /* 创建一个新的镜像文件 */
        is_new_disk = 1;    /* 一块新的磁盘，需要创建文件系统 */
        fseek(drv_file, 0, SEEK_SET);
        fwrite(drv_buf, 10, DISK_SIZE / 10,  drv_file);
        fflush(drv_file);
    }
    fseek(drv_file, 0, SEEK_END);
    filesz = ftell(drv_file);
#if DEBUG_LOCAL == 1
    printf("fatfs: disk size is %d, %d MB\n", filesz, filesz / (1024*1024));
#endif

    /* 加载磁盘数据到内存 */
    fseek(drv_file, 0, SEEK_SET);
    if (fread(drv_buf, 10, DISK_SIZE / 10, drv_file) <= 0) {
#if DEBUG_LOCAL == 1
        printf("fatfs: load disk to ram failed!\n");
#endif  
        free(drv_buf);
        fclose(drv_file);
        return -1;
    }
    open_ref++;
    return 0;
}

int drv_close()
{
    open_ref--;
    if (open_ref > 0) {
        return 0;
    }
    fseek(drv_file, 0, SEEK_SET);
    fwrite(drv_buf, DISK_SIZE / 10, 10, drv_file);
    free(drv_buf);
    return fclose(drv_file); 
}

int drv_read(unsigned int off, void *buffer, size_t size)
{
    if (off + size / SECTOR_SIZE > DISK_SECTORS)
        return -1;
    unsigned char *pos;
    pos = (unsigned char *) drv_buf + off * SECTOR_SIZE;
    memcpy(buffer, pos, size);
    return size;
}

int drv_write(unsigned int off, void *buffer, size_t size)
{
    if (off + size / SECTOR_SIZE > DISK_SECTORS)
        return -1;
    unsigned char *pos;
    pos = (unsigned char *) drv_buf + off * SECTOR_SIZE;
    memcpy(pos, buffer, size);
    return size;
}

int drv_ioctl(int cmd, unsigned long arg)
{
    switch(cmd)
    {
    case DISKIO_SYNC:
        fseek(drv_file, 0, SEEK_SET);
        fwrite(drv_buf, DISK_SIZE / 10, 10, drv_file);
        fflush(drv_file);
        break;
    case DISKIO_GETSSIZE:
        *(unsigned int *)arg = SECTOR_SIZE;
        break;     
    case DISKIO_GETSIZE:
        *(unsigned int *)arg = DISK_SECTORS;
        break;
    default:
        return -1;
    }
    return 0;
}
