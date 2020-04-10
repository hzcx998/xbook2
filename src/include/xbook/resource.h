#ifndef _XBOOK_RESOURCE_H
#define _XBOOK_RESOURCE_H

#include "types.h"
#include "stddef.h"

/* 每个进程可持有的资源数量 */
#define RES_NR    64

typedef struct {
   dev_t devno;      /* 设备号 */
   off_t off;        /* 资源偏移：针对于磁盘设备 */
} res_item_t;

typedef struct {
   res_item_t table[RES_NR];   /* 资源储存表 */
} resource_t;

#define IS_BAD_RES(res) \
      (res < 0 || res >= RES_NR)

void resource_init(resource_t *res);
void resource_copy(resource_t *res);
void resource_release(resource_t *res);

void dump_resource(resource_t *res);

int sys_getres(char *resname, unsigned long resflg);
int sys_putres(int res);
int sys_readres(int res, off_t off, void *buffer, size_t count);
int sys_writeres(int res, off_t off, void *buffer, size_t count);
int sys_ctlres(int res, unsigned int cmd, unsigned long arg);

#endif  /* _XBOOK_RESOURCE_H */
