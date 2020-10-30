#ifndef _X86_MEMPOOL_H
#define _X86_MEMPOOL_H

#include "phymem.h"
#include <xbook/list.h>

#define MEM_RANGE_NR    2

#define MEM_SECTION_MAX_SIZE    (10 * PAGE_SIZE)

typedef struct {
    mem_node_t node;
} mem_section_t;

typedef struct {
    unsigned int start;
    unsigned int end;
    size_t pages;

} mem_range_t;

void mem_range_init(unsigned int idx, unsigned int start, unsigned int end);

#endif /*_X86_MEMPOOL_H */
