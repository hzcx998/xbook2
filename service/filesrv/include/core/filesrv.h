#ifndef _FILESRV_CORE_H
#define _FILESRV_CORE_H

/* 当前服务的名字 */
#define SRV_NAME    "filesrv"

int init_srvcore();

int filesrv_create_files();
int filesrv_execute();

#endif  /* _FILESRV_CORE_H */