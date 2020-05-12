#ifndef _SYS_SRVCALL_H
#define _SYS_SRVCALL_H

#define SRV_FS      0
#define SRV_NET     1

#define SRVIO_SERVICE      0    /* 流向服务 */
#define SRVIO_USER         1    /* 流向用户 */

#define SRVARG_NR  8

#define GETSRV_DATA(arg, idx, type) \
        ((type) ((arg)->data[(idx)]))

#define GETSRV_SIZE(arg, idx) \
        ((arg)->size[(idx)])

#define SETSRV_DATA(arg, idx, value) \
        (arg)->data[(idx)] = (unsigned long) (value)
#define SETSRV_SIZE(arg, idx, value) \
        (arg)->size[(idx)] = (unsigned long) (value)

#define SETSRV_RETVAL(arg, value) \
        (arg)->retval =  (unsigned long) (value)
#define GETSRV_RETVAL(arg, type) \
        ((type) ((arg)->retval))

#define SETSRV_ARG(arg, idx, value, len) \
        (arg)->data[(idx)] = (unsigned long) (value); (arg)->size[(idx)] = (len)

#define SETSRV_IO(arg, iobits) \
        (arg)->io = (iobits)
#define GETSRV_IO(arg, type) \
        ((type) ((arg)->io))

typedef struct _srvarg {
    unsigned long data[SRVARG_NR];  /* 参数的值 */
    long size[SRVARG_NR];           /* 参数的大小 */
    /* 参数io：有8位，
    某位置1，表示对应的参数是用户流向，需要把数据读取回用户进程
    某位置0，表示对应的参数是服务流向，需要把数据写入服务进程 */
    unsigned char io;               
    unsigned long retval;           /* 返回值 */
} srvarg_t;

#define DEFINE_SRVARG(name) \
        srvarg_t name; \
        memset(&(name), 0, sizeof(srvarg_t))

int srvcall_bind(int port);
int srvcall_unbind(int port);
int srvcall_listen(int port, srvarg_t *arg);
int srvcall_ack(int port, srvarg_t *arg);
int srvcall(int port, srvarg_t *arg);
int srvcall_fetch(int port, srvarg_t *arg);
int srvcall_inbuffer(srvarg_t *arg);

/* file server call */
enum filesrv_call_num {
    FILESRV_OPEN = 0,
    FILESRV_CLOSE,
    FILESRV_READ,
    FILESRV_WRITE,
    FILESRV_LSEEK,
    FILESRV_CALL_NR,    /* 最大数量 */
};
/* 缓冲区最大长度 */
#define FILESRV_BUF_MAX_SIZE    (128*1024)

#endif   /* _SYS_SRVCALL_H */