#ifndef _GUISRV_H
#define _GUISRV_H

#include <sys/srvcall.h>

#include <stddef.h>

/* 当前服务的名字 */
#define SRV_NAME    "guisrv"

#define srvprint(...) \
        printf("[%s] %s: %s: ", SRV_NAME, __FILE__, __func__); printf(__VA_ARGS__)

typedef int (*guisrv_func_t) (srvarg_t *);

void *gui_malloc(size_t size);
void gui_free(void *ptr);
 
int init_guisrv_interface();

#endif  /* _GUISRV_H */