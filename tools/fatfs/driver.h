#ifndef _DISK_DRIVER_DEFINED
#define _DISK_DRIVER_DEFINED

int drv_init(char *path, int disk_sz);
int drv_open();
int drv_close();
int drv_ioctl(int cmd, unsigned long arg);
int drv_write(unsigned int off, void *buffer, size_t count);
int drv_read(unsigned int off, void *buffer, size_t count);

#define DISKIO_GETSIZE      1   /* 获取磁盘扇区数 */
#define DISKIO_GETSSIZE     2   /* 获取扇区大小 */
#define DISKIO_SYNC         3   /* 同步磁盘到文件 */

#endif  /* _DISK_DRIVER_DEFINED */
