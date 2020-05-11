#ifndef _SYS_SRVCALL_H
#define _SYS_SRVCALL_H

#define SRV_FS      0
#define SRV_NET     1

#define SRVIO_SERVICE      0    /* 流向服务 */
#define SRVIO_USER         1    /* 流向用户 */

#define SRVARG_NR  8

typedef struct _srvarg {
    unsigned long data[SRVARG_NR];  /* 参数的值 */
    long size[SRVARG_NR];           /* 参数的大小 */
    /* 参数io：有8位，
    某位置1，表示对应的参数是用户流向，需要把数据读取回用户进程
    某位置0，表示对应的参数是服务流向，需要把数据写入服务进程 */
    unsigned char io;               
    unsigned long retval;           /* 返回值 */
} srvarg_t;


int srvcall_bind(int port);
int srvcall_unbind(int port);
int srvcall_listen(int port, srvarg_t *arg);
int srvcall_ack(int port, srvarg_t *arg);
int srvcall(int port, srvarg_t *arg);
int srvcall_fetch(int port, srvarg_t *arg);
int srvcall_check(srvarg_t *arg);

/* file server call */
enum filesrv_call_num {
    FILESRV_OPEN = 0,
    FILESRV_CLOSE,
    FILESRV_READ,
    FILESRV_WRITE,
    FILESRV_LSEEK,
    FILESRV_CALL_NR,    /* 最大数量 */
};


#endif   /* _SYS_SRVCALL_H */