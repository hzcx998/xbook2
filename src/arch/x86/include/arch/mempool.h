#ifndef _X86_MEMPOOL_H
#define _X86_MEMPOOL_H

#include "phymem.h"
#include <xbook/list.h>

enum {
    MEM_RANGE_DMA = 0,
    MEM_RANGE_NORMAL,
    MEM_RANGE_USER,
    MEM_RANGE_NR
};

#define MEM_SECTION_MAX_NR      10

#define MEM_SECTION_MAX_SIZE    512    // 2 ^ 9

typedef struct {
    unsigned int start;
    unsigned int end;
    size_t pages;
    mem_section_t sections[MEM_SECTION_MAX_NR];
    mem_node_t *node_table;     // 节点表
} mem_range_t;

void mem_range_init(unsigned int idx, unsigned int start, size_t len);

void mem_pool_test();

#endif /*_X86_MEMPOOL_H */
