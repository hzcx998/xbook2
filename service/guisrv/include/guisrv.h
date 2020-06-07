#ifndef _GUISRV_H
#define _GUISRV_H

#include <sys/srvcall.h>

#include <stddef.h>

/* 当前服务的名字 */
#define SRV_NAME    "guisrv"

void *gui_malloc(size_t size);
void gui_free(void *ptr);

#endif  /* _GUISRV_H */