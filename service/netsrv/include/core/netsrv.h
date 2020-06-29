
#ifndef __NETSRV_H
#define __NETSRV_H

/* 当前服务的名字 */
#define SRV_NAME    "netsrv"

void lwip_init_task(void);

#define srvprint(...) \
        printf("[%s] %s: %s: ", SRV_NAME, __FILE__, __func__); printf(__VA_ARGS__)

#endif  /* __NETSRV_H */
