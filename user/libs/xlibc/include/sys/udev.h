#ifndef _SYS_UDEV_H
#define _SYS_UDEV_H

#define DEVICE_NAME_LEN 32

/* 设备项 */
typedef struct __devent {
    short de_type;      /* device type */
    char de_name[DEVICE_NAME_LEN];   /* device name */
} devent_t;

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

int scandev(devent_t *de, device_type_t type, devent_t *out_de);

#endif   /* _SYS_UDEV_H */