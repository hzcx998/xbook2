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

/* 定义系统的设备控制码 */

/* 控制台 */
#define CONIO_CLEAR      DEVCTL_CODE('c', 1)
#define CONIO_SCROLL     DEVCTL_CODE('c', 2)
#define CONIO_SETCOLOR   DEVCTL_CODE('c', 3)
#define CONIO_GETCOLOR   DEVCTL_CODE('c', 4)
#define CONIO_SETPOS     DEVCTL_CODE('c', 5)
#define CONIO_GETPOS     DEVCTL_CODE('c', 6)

/* disk */
#define DISKIO_GETSIZE   DEVCTL_CODE('d', 1)
#define DISKIO_CLEAR     DEVCTL_CODE('d', 2)

/* tty */
#define TTYIO_CLEAR      CONIO_CLEAR
#define TTYIO_SCROLL     CONIO_SCROLL
#define TTYIO_SETCOLOR   CONIO_SETCOLOR
#define TTYIO_GETCOLOR   CONIO_GETCOLOR
#define TTYIO_SETPOS     CONIO_SETPOS
#define TTYIO_GETPOS     CONIO_GETPOS
#define TTYIO_HOLDER     DEVCTL_CODE('t', 1)
#define TTYIO_VISITOR    DEVCTL_CODE('t', 2)

#endif   /* _SYS_IOCTL_H */