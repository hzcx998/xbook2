#ifndef _SYS_SRVCALL_H
#define _SYS_SRVCALL_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * 系统调用是用户态陷入内核，让内核帮忙执行一些内容。
 * 服务调用是用户态转移到用户态，让服务进程帮忙执行一些内容。
 * srvcall, 传入参数，然后先陷入内核，然后寻找到对应的服务进程。
 * 查看服务进程的状态，如果可转移，那么就直接转移，不然，就等待。
 * 
 * 服务进程，先通过系统调用，进入到可服务状态，等待其它进程跳转服务。
 * 先查看就绪队列，如果有就从里面挑选一个出来，并执行。然后继续陷入
 * 内核挑选，直到就绪队列为空，然后就进入阻塞等待状态。
 * 
 * 服务进程：处于可转移状态，当第一个进程跳转的时候，将参数传递给它
 * 然后唤醒它。然后阻塞调用进程。
 * 处于不可转移状态，说明正在处理一个转移，那么，就需要把当前进程挂到
 * 服务进程的就绪队列，然后阻塞自己，等待服务进程处理完服务后并解除
 * 阻塞。
 */

#define SRV_FS      0
#define SRV_NET     1
#define SRV_GUI     2

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

#ifdef __cplusplus
}
#endif

#endif   /* _SYS_SRVCALL_H */