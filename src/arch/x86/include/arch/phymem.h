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

int physic_memory_init();

unsigned long mem_get_free_page_nr();
unsigned long mem_get_total_page_nr();

#endif   /*_X86_PHYMEM_H */
