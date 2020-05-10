#ifndef _SYS_SRVCALL_H
#define _SYS_SRVCALL_H

#define SRV_FS      0
#define SRV_NET     1

/* 参数个数 */
#define SRVCALL_ARG_NR  5

typedef struct _srvcall_arg {
    unsigned long data[SRVCALL_ARG_NR];
} srvcall_arg_t;

int srvcall_bind(int port);
int srvcall_unbind(int port);
int srvcall_listen(int port, srvcall_arg_t *arg);
int srvcall_ack(int port, srvcall_arg_t *arg);
int srvcall(int port, srvcall_arg_t *arg);

/* file server call */
enum filesrv_call_num {
    FILESRV_OPEN = 0,
    FILESRV_CLOSE,
    FILESRV_READ,
    FILESRV_WRITE,
    FILESRV_LSEEK,
};


#endif   /* _SYS_SRVCALL_H */