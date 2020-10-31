#ifndef _X86_PHYMEM_H
#define _X86_PHYMEM_H

#include "atomic.h"
#include <xbook/memcache.h>
#include <const.h>

// 0MB~1MB是体系结构相关的内存分布
#define BIOS_MEM_ADDR               0X000000000
#define BIOS_MEM_SIZE               (1 * MB)

#define KERNEL_SELF_MEM_ADDR        (BIOS_MEM_ADDR+BIOS_MEM_SIZE)
#define KERNEL_SELF_MEM_SIZE        (1 * MB)

#define KERN_DATA_MEM_ADDR          (KERNEL_SELF_MEM_ADDR+KERNEL_SELF_MEM_SIZE)
#define KERN_DATA_MEM_SIZE          (6 * MB)

#define DMA_MEM_ADDR                (KERN_DATA_MEM_ADDR+KERN_DATA_MEM_SIZE)
#define DMA_MEM_SIZE                (8 * MB)

#define NORMAL_MEM_ADDR             (DMA_MEM_ADDR+DMA_MEM_SIZE) 
#define TOP_MEM_ADDR                0xFFFFFFFF 

/* 空内存，当前页目录表物理地址的映射（不可访问） */
#define KERN_BLACKHOLE_MEM_SIZE            (4 * MB)
#define KERN_LIMIT_MEM_ADDR                (TOP_MEM_ADDR - KERN_BLACKHOLE_MEM_SIZE + 1)

#define DYNAMIC_MAP_MEM_SIZE               (128 * MB)
#define DYNAMIC_MAP_MEM_ADDR               (TOP_MEM_ADDR - (DYNAMIC_MAP_MEM_SIZE + KERN_BLACKHOLE_MEM_SIZE) + 1)

#define DYNAMIC_MAP_MEM_END    KERN_LIMIT_MEM_ADDR

#define KERNEL_STATCK_TOP		0x8009f000
#define KERNEL_STATCK_BOTTOM	(KERNEL_STATCK_TOP - (TASK_KSTACK_SIZE))

#define MEM_NODE_TYPE_DMA        0x01
#define MEM_NODE_TYPE_NORMAL     0x02


typedef struct {
    list_t node_list_head;
    size_t node_count;
} mem_section_t;

typedef struct _mem_node {
    unsigned int count;         /* 内存节点占用的页数 */
    unsigned int flags;         
    int reference;              /* 引用次数 */
    list_t list;                /* 节点链表 */
    mem_cache_t *cache;         /* 内存缓冲 */
    mem_group_t *group;         /* 内存组 */
} mem_node_t;



#define SIZEOF_MEM_NODE sizeof(mem_node_t) 

#define MEM_NODE_MARK_CHACHE_GROUP(node, cache, group)  \
        node->cache = cache;                            \
        node->group = group
        
#define MEM_NODE_CLEAR_GROUP_CACHE(node)                \
        node->cache = NULL;                          \
        node->group = NULL
        
#define MEM_NODE_GET_GROUP(node) node->group
#define MEM_NODE_GET_CACHE(node) node->cache

#define CHECK_MEM_NODE(node)                            \
        if (node == NULL) panic("Mem node error!\n") 

int physic_memory_init();

void dump_mem_node(mem_node_t *node);

mem_node_t *phy_addr_to_mem_node(unsigned int page);
unsigned int mem_node_to_phy_addr(mem_node_t *node);

unsigned long mem_get_free_page_nr();
unsigned long mem_get_total_page_nr();

unsigned long mem_node_alloc_pages(unsigned long count, unsigned long flags);
int mem_node_free_pages(unsigned long page);

#endif   /*_X86_PHYMEM_H */
