#ifndef _SYS_VMM_H
#define _SYS_VMM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

/* 物理内存信息 */
typedef struct {
    unsigned long ms_total;    /* 物理内存总大小 */
    unsigned long ms_free;     /* 物理内存空闲大小 */
    unsigned long ms_used;     /* 物理内存已使用大小 */
} mstate_t;

int mstate(mstate_t *ms);

/* vmm */
void *heap(void *heap);
int munmap(void *addr, size_t length);

#ifdef __cplusplus
}
#endif

#endif  /* _SYS_VMM_H */
