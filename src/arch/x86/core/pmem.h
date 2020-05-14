#ifndef _X86_PMEM_H
#define _X86_PMEM_H

#include "atomic.h"
#include <xbook/memcache.h>

// 0MB~1MB是体系结构相关的内存分布
#define BIOS_MEM_ADDR               0X000000000
#define BIOS_MEM_SIZE               0X000100000     // 1MB

// 1MB~2MB是内核镜像
#define KERNEL_MEM_ADDR             (BIOS_MEM_ADDR+BIOS_MEM_SIZE)
#define KERNEL_MEM_SIZE             0X000100000     // 1MB

// 2MB~8MB是系统重要信息的存放地址
#define MATERIAL_MEM_ADDR           (KERNEL_MEM_ADDR+KERNEL_MEM_SIZE)
#define MATERIAL_MEM_SIZE           0X000600000   // 6MB

// 8MB~16MB是设备DMA的地址
#define DMA_MEM_ADDR                (MATERIAL_MEM_ADDR+MATERIAL_MEM_SIZE)
#define DMA_MEM_SIZE                0X000800000   // 10MB

// ---- 以上是必须的基本内存使用情况 ----

// 16M以上是普通内存开始地址
#define NORMAL_MEM_ADDR             (DMA_MEM_ADDR+DMA_MEM_SIZE) 

#define TOP_MEM_ADDR                0xFFFFFFFF // 最高内存地址

/* 空内存，当前页目录表物理地址的映射（不可访问） */
#define NULL_MEM_SIZE                0X400000
#define NULL_MEM_ADDR                (TOP_MEM_ADDR - NULL_MEM_SIZE + 1)

#define HIGH_MEM_SIZE               0x8000000 // 非连续性内存128MB

/* 非连续内存起始地址, 4MB+128MB*/
#define HIGH_MEM_ADDR               (TOP_MEM_ADDR - (HIGH_MEM_SIZE + NULL_MEM_SIZE) + 1)

#define __VMAREA_BASE   HIGH_MEM_ADDR
#define __VMAREA_END    NULL_MEM_ADDR



/* mem_node 内存节点，用于管理每一段物理内存（以页为单位） */
typedef struct {
    unsigned int count;         /* 内存节点占用的页数 */
    unsigned int flags;         /* 节点标志 */
    int reference;     /* 引用次数 */
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

int init_pmem();

void dump_mem_node(mem_node_t *node);

mem_node_t *__page2mem_node(unsigned int page);
unsigned int __mem_node2page(mem_node_t *node);

mem_node_t *get_free_mem_node();

unsigned long __get_free_page_nr();
unsigned long __get_total_page_nr();

#endif   /*_X86_PMEM_H */
