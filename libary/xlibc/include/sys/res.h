#ifndef _SYS_RES_H
#define _SYS_RES_H

#include <types.h>


/* 资源类型 */
#define RES_DEV    0x1000000
#define RES_IPC    0x2000000

/* 标准输入输出资源 */
#define RES_STDINNO   0
#define RES_STDOUTNO   1
#define RES_STDERRNO   2

/* 资源的控制命令 */
#define RES_REDIR    1

#define DEVICE_NAME_LEN 32

#define SECTOR_SIZE     512

typedef enum _device_type {
    DEVICE_TYPE_BEEP,                    /* 蜂鸣器设备 */
    DEVICE_TYPE_DISK,                    /* 磁盘设备 */
    DEVICE_TYPE_KEYBOARD,                /* 键盘设备 */
    DEVICE_TYPE_MOUSE,                   /* 鼠标设备 */
    DEVICE_TYPE_NULL,                    /* 空设备 */
    DEVICE_TYPE_PORT,                   /* 端口设备 */
    DEVICE_TYPE_SERIAL_PORT,            /* 串口设备 */
    DEVICE_TYPE_PARALLEL_PORT,           /* 并口设备 */
    DEVICE_TYPE_PHYSIC_NETCARD,          /* 物理网卡设备 */
    DEVICE_TYPE_PRINTER,                 /* 打印机设备 */
    DEVICE_TYPE_SCANNER,                 /* 扫描仪设备 */
    DEVICE_TYPE_SCREEN,                  /* 屏幕设备 */
    DEVICE_TYPE_SOUND,                   /* 声音设备 */
    DEVICE_TYPE_STREAM,                  /* 流设备 */
    DEVICE_TYPE_UNKNOWN,                 /* 未知设备 */
    DEVICE_TYPE_VIDEO,                   /* 视频设备 */
    DEVICE_TYPE_VIRTUAL_DISK,            /* 虚拟磁盘设备 */
    DEVICE_TYPE_VIRTUAL_CHAR,            /* 虚拟字符设备 */
    DEVICE_TYPE_WAVE_IN,                 /* 声音输入设备 */
    DEVICE_TYPE_WAVE_OUT,                /* 声音输出设备 */
    DEVICE_TYPE_8042_PORT,               /* 8042端口设备 */
    DEVICE_TYPE_NETWORK,                 /* 网络设备 */
    DEVICE_TYPE_BUS_EXTERNDER,           /* BUS总线扩展设备 */
    DEVICE_TYPE_ACPI,                    /* ACPI设备 */
    MAX_DEVICE_TYPE_NR
} device_type_t;

/* 设备项 */
typedef struct __devent {
    short de_type;      /* device type */
    char de_name[DEVICE_NAME_LEN];   /* device name */
} devent_t;

int res_open(char *name, unsigned long flags, unsigned long arg);
int res_close(int res);
int res_write(int res, off_t off, void *buffer, size_t size);
int res_read(int res, off_t off, void *buffer, size_t size);
int res_ioctl(int res, unsigned int cmd, unsigned long arg);
int dev_scan(devent_t *de, device_type_t type, devent_t *out);
void *res_mmap(int res, size_t length, int flags);
unid_t res_unid(int id);
int res_redirect(int old_res, int new_res);

/* 磁盘驱动器 */
typedef struct disk_drive {
    int handle;         /* 句柄 */
    char seq[3];        /* 序列号 */
    devent_t devent;    /* 设备项 */
} disk_drive_t;

#endif   /* _SYS_RES_H */