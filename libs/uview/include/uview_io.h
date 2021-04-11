#ifndef _LIB_UVIEW_IO_H
#define _LIB_UVIEW_IO_H

#include <sys/ioctl.h>

#ifdef __cplusplus
extern "C" {
#endif

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
#define VIEWIO_SETMONITOR   DEVCTL_CODE('v', 26)
#define VIEWIO_SETWINMAXIMRECT   DEVCTL_CODE('v', 27)
#define VIEWIO_GETWINMAXIMRECT   DEVCTL_CODE('v', 28)

#ifdef __cplusplus
}
#endif

#endif  /* _LIB_UVIEW_IO_H */