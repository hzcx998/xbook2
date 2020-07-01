
#ifndef __NETSRV_H
#define __NETSRV_H

#include <sys/srvcall.h>
#include <srv/netsrv.h>

/* 当前服务的名字 */
#define SRV_NAME    "netsrv"

void lwip_init_task(void);

#define srvprint(...) \
        printf("[%s] %s: %s: ", SRV_NAME, __FILE__, __func__); printf(__VA_ARGS__)


typedef int (*srvcall_func_t) (srvarg_t *);

extern srvcall_func_t netsrv_call_table[];

#endif  /* __NETSRV_H */
