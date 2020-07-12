#ifndef _XBOOK_RESOURCE_H
#define _XBOOK_RESOURCE_H

#include <types.h>
#include <stddef.h>

/* 每个进程可持有的资源数量 */
#define RES_NR    64

#define RES_MASK  0x00ffffff

/* 资源本身的标志，0~15位 */
#define RES_LOCAL_MASK  0x0000ffff
/* 资源管理类型的标志，24~31是主类型 */
#define RES_MASTER_MASK  0xff000000
/* 资源管理类型的标志，16~23位是从类型 */
#define RES_SLAVER_MASK  0x00ff0000
/* 资源管理类型的标志，16~31位，16~23位是从类型，24~31是主类型 */
#define RES_GLOBAL_MASK  (RES_MASTER_MASK | RES_SLAVER_MASK) 


typedef struct {
   unsigned long flags;    /* 打开时的资源类型 */
   unsigned long handle;   /* 资源句柄 */
} res_item_t;

typedef struct {
   res_item_t table[RES_NR];   /* 资源储存表 */
} resource_t;

#define IS_BAD_RES(res) \
      (res < 0 || res >= RES_NR)

void resource_init(resource_t *res);
void resource_copy(resource_t *dst, resource_t *src);
void resource_release(resource_t *res);
int resource_item_copy(int dst_res, int src_res);

void dump_resource(resource_t *res);

int sys_getres(char *resname, unsigned long resflg, unsigned long arg);
int sys_putres(int res);
int sys_readres(int res, off_t off, void *buffer, size_t count);
int sys_writeres(int res, off_t off, void *buffer, size_t count);
int sys_ctlres(int res, unsigned int cmd, unsigned long arg);
void *sys_mmap(int res, size_t length, int flags);
unsigned long sys_unid(int id);

#endif  /* _XBOOK_RESOURCE_H */
