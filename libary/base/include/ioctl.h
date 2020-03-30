#ifndef _SYS_IOCTL_H
#define _SYS_IOCTL_H

/* ioctl format */


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

#endif   /* _SYS_IOCTL_H */