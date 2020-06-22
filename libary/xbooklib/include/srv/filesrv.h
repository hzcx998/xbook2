#ifndef _SRV_FILE_SRV_H
#define _SRV_FILE_SRV_H

/* file server call */
enum filesrv_call_num {
    FILESRV_OPEN = 0,
    FILESRV_CLOSE,
    FILESRV_READ,
    FILESRV_WRITE,
    FILESRV_LSEEK,
    FILESRV_ASSERT,
    FILESRV_CALL_NR,    /* 最大数量 */
};
/* 缓冲区最大长度 */
#define FILESRV_BUF_MAX_SIZE    (128*1024)


#define MAX_PATH_LEN    260


#endif   /* _SRV_FILE_SRV_H */