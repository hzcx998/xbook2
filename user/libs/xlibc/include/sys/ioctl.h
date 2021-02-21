#ifndef _SYS_IOCTL_H
#define _SYS_IOCTL_H

#ifdef __cplusplus
extern "C" {
#endif

/* 设备控制码：
0~15位：命令（0-0x7FFF系统保留，0x8000-0xffff用户自定义）
16~31位：设备类型
 */
#ifndef DEVCTL_CODE
#define DEVCTL_CODE(type, cmd) \
        ((unsigned int) ((((type) & 0xffff) << 16) | ((cmd) & 0xffff)))
#endif

/* 设备标志 */
#define DEV_NOWAIT      0x01        /* 非阻塞方式 */


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
#define DISKIO_SETOFF    DEVCTL_CODE('d', 3)
#define DISKIO_GETOFF    DEVCTL_CODE('d', 4)

/* tty */
#define TTYIO_CLEAR         CONIO_CLEAR
#define TTYIO_SCROLL        CONIO_SCROLL
#define TTYIO_SETCOLOR      CONIO_SETCOLOR
#define TTYIO_GETCOLOR      CONIO_GETCOLOR
#define TTYIO_SETPOS        CONIO_SETPOS
#define TTYIO_GETPOS        CONIO_GETPOS
#define TTYIO_HOLDER        DEVCTL_CODE('t', 1)
#define TTYIO_SELECT        DEVCTL_CODE('t', 2)
#define TIOCGPTN            DEVCTL_CODE('t', 5) /* get presudo tty number */
#define TIOCSPTLCK          DEVCTL_CODE('t', 6) /* set presudo tty lock */
#define TIOCSFLGS           DEVCTL_CODE('t', 7) /* set flags */
#define TIOCGFLGS           DEVCTL_CODE('t', 8) /* get flags */
#define TIOCGFG             DEVCTL_CODE('t', 9) /* get front group task */
#define TIOCISTTY           DEVCTL_CODE('t', 10) /* check is tty */
#define TIOCNAME            DEVCTL_CODE('t', 11) /* get tty name */
#define TTYIO_RAW           7

/* tty flags */
#define TTYFLG_ECHO    0x01

/* net */
#define NETIO_GETMAC        DEVCTL_CODE('n', 1)
#define NETIO_SETMAC        DEVCTL_CODE('n', 2)
#define NETIO_SETFLGS       DEVCTL_CODE('n', 3)
#define NETIO_GETFLGS       DEVCTL_CODE('n', 4)


/* video */
typedef struct _video_info {
    char bits_per_pixel;                  /* 每个像素的位数 */
    short bytes_per_scan_line;          /* 单行的字节数 */
    short x_resolution, y_resolution;   /* 分辨率x，y */    
} video_info_t;
#define VIDEOIO_GETINFO     DEVCTL_CODE('v', 1) /* get video info */

/* even */
#define EVENIO_GETLED     DEVCTL_CODE('e', 1) /* get led states */
#define EVENIO_SETFLG     DEVCTL_CODE('e', 2) /* set flags */
#define EVENIO_GETFLG     DEVCTL_CODE('e', 3) /* get flags */

/* sound */
#define SNDIO_PLAY          DEVCTL_CODE('s', 1) /* play */
#define SNDIO_STOP          DEVCTL_CODE('s', 2) /* stop play */
#define SNDIO_SETFREQ       DEVCTL_CODE('s', 3) /* set play freq */

/* view */
#define VIEWIO_SHOW         DEVCTL_CODE('v', 1)
#define VIEWIO_HIDE         DEVCTL_CODE('v', 2)
#define VIEWIO_SETPOS       DEVCTL_CODE('v', 3)
#define VIEWIO_GETPOS       DEVCTL_CODE('v', 4)
#define VIEWIO_WRBMP        DEVCTL_CODE('v', 5)
#define VIEWIO_RDBMP        DEVCTL_CODE('v', 6)
#define VIEWIO_SETFLGS      DEVCTL_CODE('v', 7)
#define VIEWIO_GETFLGS      DEVCTL_CODE('v', 8)
#define VIEWIO_SETTYPE      DEVCTL_CODE('v', 9)
#define VIEWIO_GETTYPE      DEVCTL_CODE('v', 10)
#define VIEWIO_REFRESH      DEVCTL_CODE('v', 11)
#define VIEWIO_ADDATTR      DEVCTL_CODE('v', 12)
#define VIEWIO_DELATTR      DEVCTL_CODE('v', 13)
#define VIEWIO_RESIZE       DEVCTL_CODE('v', 14)
#define VIEWIO_GETSCREENSZ  DEVCTL_CODE('v', 15)
#define VIEWIO_GETLASTPOS   DEVCTL_CODE('v', 16)
#define VIEWIO_GETMOUSEPOS  DEVCTL_CODE('v', 17)
#define VIEWIO_SETSIZEMIN   DEVCTL_CODE('v', 18)
#define VIEWIO_SETDRAGREGION  DEVCTL_CODE('v', 19)
#define VIEWIO_SETMOUSESTATE  DEVCTL_CODE('v', 20)
#define VIEWIO_SETMOUSESTATEINFO  DEVCTL_CODE('v', 21)
#define VIEWIO_GETVID       DEVCTL_CODE('v', 22)
#define VIEWIO_ADDTIMER     DEVCTL_CODE('v', 23)
#define VIEWIO_DELTIMER     DEVCTL_CODE('v', 24)
#define VIEWIO_RESTARTTIMER     DEVCTL_CODE('v', 25)

#ifdef __cplusplus
}
#endif

#endif   /* _SYS_IOCTL_H */