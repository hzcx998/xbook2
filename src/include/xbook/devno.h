#ifndef _XBOOK_DEVNO_H
#define _XBOOK_DEVNO_H

/* 这里学习linux的做法，高12位是主设备号，低20位是从设备号 */
#define MINORBITS 20
#define MINORMASK 0x000fffff

#define MKDEV(major,minor) (((major) << MINORBITS) | (minor))

#define MAJOR(dev) ((dev) >> MINORBITS)
#define MINOR(dev) ((dev) & MINORMASK)

/* 设备号 */
#define NONE_MAJOR          0   /* 第一个设备号不使用 */
#define CONSOLE_MAJOR       1
#define COM_MAJOR           2
#define IDE_MAJOR           10
#define RAMDISK_MAJOR       11

#define NULL_MAJOR          0xfff   /* 最后一个设备号 */

#define CONSOLE_MINORS       8
#define COM_MINORS           2
#define IDE_MINORS           4
#define RAMDISK_MINORS       1

/* 设备号合成 */
#define DEV_NULL            MKDEV(NULL_MAJOR, 0)       /* 控制台设备0 */

#define DEV_CON0            MKDEV(CONSOLE_MAJOR, 0)       /* 控制台设备0 */
#define DEV_CON1            MKDEV(CONSOLE_MAJOR, 1)       /* 控制台设备1 */
#define DEV_CON2            MKDEV(CONSOLE_MAJOR, 2)       /* 控制台设备2 */
#define DEV_CON3            MKDEV(CONSOLE_MAJOR, 3)       /* 控制台设备3 */

#define DEV_HD0             MKDEV(IDE_MAJOR, 0)  /* IDE硬盘：主通道0磁盘 */
#define DEV_HD1             MKDEV(IDE_MAJOR, 1)  /* IDE硬盘：主通道1磁盘 */
#define DEV_HD2             MKDEV(IDE_MAJOR, 2)  /* IDE硬盘：从通道0磁盘 */
#define DEV_HD3             MKDEV(IDE_MAJOR, 3)  /* IDE硬盘：从通道1磁盘 */

#define DEV_RD0             MKDEV(RAMDISK_MAJOR, 0)   /* RAMDISK设备 */
#define DEV_RD1             MKDEV(RAMDISK_MAJOR, 1)   /* RAMDISK设备 */

#define DEV_COM1            MKDEV(COM_MAJOR, 0)       /* COM设备0 */
#define DEV_COM2            MKDEV(COM_MAJOR, 1)       /* COM设备1 */
#define DEV_COM3            MKDEV(COM_MAJOR, 2)       /* COM设备2 */
#define DEV_COM4            MKDEV(COM_MAJOR, 3)       /* COM设备3 */

#endif   /* _XBOOK_DEVNO_H */
