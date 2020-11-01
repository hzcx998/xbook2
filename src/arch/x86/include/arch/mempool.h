#ifndef _X86_MEMPOOL_H
#define _X86_MEMPOOL_H

#include "phymem.h"
#include <xbook/list.h>
#include <xbook/memcache.h>
#include <const.h>

#define MEM_NODE_TYPE_DMA        0x01
#define MEM_NODE_TYPE_NORMAL     0x02
#define MEM_NODE_TYPE_USER      0x04

enum {
    MEM_RANGE_DMA = 0,
    MEM_RANGE_NORMAL,
    MEM_RANGE_USER,
    MEM_RANGE_NR
};

#define MEM_SECTION_MAX_NR      10

#define MEM_SECTION_MAX_SIZE    512    // 2 ^ 9


typedef struct {
    list_t free_list_head;
    size_t node_count;
    size_t section_size;
} mem_section_t;

typedef struct _mem_node {
    unsigned int count;
    unsigned int flags;         
    int reference; 
    list_t list;
    mem_section_t *section;
    mem_cache_t *cache;
    mem_group_t *group;
} mem_node_t;

typedef struct {
    unsigned int start;
    unsigned int end;
    size_t pages;
    mem_section_t sections[MEM_SECTION_MAX_NR];
    mem_node_t *node_table;
} mem_range_t;

#define SIZEOF_MEM_NODE sizeof(mem_node_t) 

#define MEM_NODE_MARK_CHACHE_GROUP(node, _cache, _group)  \
        node->cache = _cache;                            \
        node->group = _group
        
#define MEM_NODE_MARK_SECTION(node, _section)  \
        node->section = _section
        
#define MEM_NODE_CLEAR_GROUP_CACHE(node)                \
        node->cache = NULL;                          \
        node->group = NULL
#define MEM_NODE_CLEAR_SECTION(node)                \
        node->section = NULL
        
#define MEM_NODE_GET_GROUP(node) node->group
#define MEM_NODE_GET_CACHE(node) node->cache
#define MEM_NODE_GET_SECTION(node) node->section

#define MEM_SECTION_DES_COUNT(section) \
        section->node_count--

#define MEM_SECTION_INC_COUNT(section) \
        section->node_count++

#define CHECK_MEM_NODE(node)                            \
        if (node == NULL) panic("Mem node error!\n") 

mem_node_t *phy_addr_to_mem_node(unsigned int page);
unsigned int mem_node_to_phy_addr(mem_node_t *node);

unsigned long mem_node_alloc_pages(unsigned long count, unsigned long flags);
int mem_node_free_pages(unsigned long page);

void mem_range_init(unsigned int idx, unsigned int start, size_t len);

void mem_pool_test();

#endif /*_X86_MEMPOOL_H */
