#ifndef _SYS_VMM_H
#define _SYS_VMM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <types.h>
#include <stddef.h>

/* 物理内存信息 */
typedef struct {
    unsigned long ms_total;    /* 物理内存总大小 */
    unsigned long ms_free;     /* 物理内存空闲大小 */
    unsigned long ms_used;     /* 物理内存已使用大小 */
} mstate_t;

int mstate(mstate_t *ms);

typedef struct {
    void *addr;
    size_t length;
    int prot;
    int flags;
    int fd;
    off_t offset;
} mmap_args_t;

/* vmm */
void *heap(void *heap);
int xmunmap(void *addr, size_t length);
void *xmmap(int fd, size_t length, int flags);

#ifdef __cplusplus
}
#endif

#endif  /* _SYS_VMM_H */
