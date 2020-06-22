#ifndef _FILESRV_CORE_INTERFACE_H
#define _FILESRV_CORE_INTERFACE_H

#include <sys/srvcall.h>
#include <srv/filesrv.h>
#include <ff.h>

typedef int (*srvcall_func_t) (srvarg_t *);

extern srvcall_func_t filesrv_call_table[];

int init_srv_interface();

#endif  /* _FILESRV_CORE_INTERFACE_H */