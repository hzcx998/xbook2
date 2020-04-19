#ifndef _SYS_IOCTL_H
#define _SYS_IOCTL_H

/* 设备控制码：
0~15位：命令（0-0x7FFF系统保留，0x8000-0xffff用户自定义）
16~31位：设备类型
 */
#ifndef DEVCTL_CODE
#define DEVCTL_CODE(type, cmd) \
        ((unsigned int) ((((type) & 0xffff) << 16) | ((cmd) & 0xffff)))
#endif

/* drivers */

/* console */
enum ioctl_console {
    CONIO_SETCOLOR = 1,
    CONIO_SCROLL,
    CONIO_CLEAN,
    CONIO_SETCURSOR,
};

/* ide */
enum ioctl_ide {
    IDEIO_RINSE = 1,
    IDEIO_GETCNTS,
};

/* 定义系统的设备控制码 */
#define CODE_CON_CLEAR      DEVCTL_CODE('c', 1)
#define CODE_CON_SCROLL     DEVCTL_CODE('c', 2)
#define CODE_CON_SETCOLOR   DEVCTL_CODE('c', 3)
#define CODE_CON_GETCOLOR   DEVCTL_CODE('c', 4)
#define CODE_CON_SETPOS     DEVCTL_CODE('c', 5)
#define CODE_CON_GETPOS     DEVCTL_CODE('c', 6)

#define CODE_DISK_GETSIZE   DEVCTL_CODE('d', 1)
#define CODE_DISK_CLEAR     DEVCTL_CODE('d', 2)



#endif   /* _SYS_IOCTL_H */