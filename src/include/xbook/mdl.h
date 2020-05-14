#ifndef _XBOOK_MDL_H
#define _XBOOK_MDL_H

#include "const.h"
#include "task.h"
#include "driver.h"

#define MDL_MAX_SIZE    (32 * MB)

/* memory description list(MDL) 内存描述链表 */
typedef struct _mdl {
    struct _mdl *next;      /* 下一个mdl */
    task_t *task;           /* 所在的任务 */
    void *mapped_vaddr;     /* 映射后的地址，整页 */
    void *start_vaddr;      /* 开始的虚拟地址，整页 */
    unsigned long byte_count;   /* 映射的字节数 */
    unsigned long byte_offset;   /* 映射的字节数 */
} mdl_t;

#define MDL_GET_BYTE_COUNT(mdl) ((mdl)->byte_count)
#define MDL_GET_BYTE_OFFSET(mdl) ((mdl)->byte_offset)
#define MDL_GET_START_VADDR(mdl) ((mdl)->start_vaddr + (mdl)->byte_offset)
#define MDL_GET_MAPPED_VADDR(mdl) ((mdl)->mapped_vaddr + (mdl)->byte_offset)

mdl_t *mdl_alloc(void *vaddr, unsigned long length,
    bool later, io_request_t *ioreq);

void mdl_free(mdl_t *mdl);



#endif /* _XBOOK_MDL_H */
