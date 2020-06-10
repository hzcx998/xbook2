#ifndef _XBOOK_SHAREMEM_H
#define _XBOOK_SHAREMEM_H

#include "const.h"
#include "../arch/atomic.h"

/* 最多有多少个共享内存 */
#define MAX_SHARE_MEM_NR        128
/* 单个共享内存最大大小 */
#define MAX_SHARE_MEM_SIZE      (8 * MB)

#define SHARE_MEM_NAME_LEN      24

/* 共享内存结构 */
typedef struct share_mem {
    unsigned short id;          /* 共享内存id */
    unsigned long page_addr;    /* 共享的物理内存 */
    unsigned long npages;       /* 物理页数量 */
    atomic_t links;        /* 使用这段共享内存被映射的次数 */
    char name[SHARE_MEM_NAME_LEN];      /* 名字 */
} share_mem_t;

share_mem_t *share_mem_alloc(char *name, unsigned long size);
int share_mem_free(share_mem_t *shm);

int share_mem_get(char *name, unsigned long size, unsigned long flags);
int share_mem_put(int shmid);

void *share_mem_map(int shmid, void *shmaddr, int shmflg);
int share_mem_unmap(const void *shmaddr, int shmflg);

void init_share_mem();



#endif   /* _XBOOK_SHAREMEM_H */
