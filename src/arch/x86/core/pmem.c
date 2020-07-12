#include "pmem.h"
#include "page.h"
#include "bootmem.h"
#include <xbook/debug.h>
#include <math.h>
#include <string.h>

extern unsigned int get_memory_size_from_hardware();

/*
申请一个内存节点数组，通过引用来表明是否被使用
指向内存节点数组的指针
*/
mem_node_t *mem_node_table;
/* 节点数量 */
unsigned int mem_node_count;
/* 节点基地址 */
unsigned int mem_node_base;

/* 物理内存总大小 */
static unsigned long total_pmem_size;

mem_node_t *get_free_mem_node()
{
    mem_node_t *node = mem_node_table;
    
    while (node < mem_node_table + mem_node_count)
    {
        /* 没有被分配就是我们需要的 */
        if (!node->reference) {
            return node;
        }
        /* 指向下一个节点位置 */
        if (node->count) 
            node += node->count;
        else 
            node++;   
    }
    return NULL;
}

mem_node_t *__page2mem_node(unsigned int page)
{ 
    int index = (page - mem_node_base) >> PAGE_SHIFT;

    mem_node_t *node = mem_node_table + index;

    /* 必须是在正确的范围 */
    if (node < mem_node_table || node >= mem_node_table + mem_node_count)
        return NULL;

    return node;
}

unsigned int __mem_node2page(mem_node_t *node)
{ 
    int index = node - mem_node_table;

    return mem_node_base + (index << PAGE_SHIFT);
}

void dump_mem_node(mem_node_t *node)
{ 
    printk("----Mem Node----\n");
    printk("count: %d flags:%x reference:%d\n",
        node->count, node->flags, node->reference);
    if (node->cache && node->group) {
        printk("cache: %x group:%x\n",
            node->cache, node->group);
    }
}

/* 
 * ZoneCutUesdMemory - 把已经使用的内存剪掉
 * 
 * 因为内存管理器本身要占用一定内存，在这里把它从可分配中去掉
 */
static void cut_used_mem()
{
    /* 剪切掉引导分配的空间 */
    unsigned int used_mem = boot_mem_size();
    unsigned int used_pages = DIV_ROUND_UP(used_mem, PAGE_SIZE);
    __alloc_pages(used_pages);
}

/** 
 * __get_free_page_nr - 获取物理内存空闲页数
 * 
 * 返回物理空闲页数
 */
unsigned long __get_free_page_nr()
{
    mem_node_t *node = mem_node_table;
    unsigned long free_nodes = 0;
    while (node < mem_node_table + mem_node_count)
    {
        /* 没有被分配就是我们需要的 */
        if (!node->reference) {
            free_nodes++;
        }
        /* 指向下一个节点位置 */
        if (node->count) 
            node += node->count;
        else 
            node++;
    }
    /* 获取空闲节点数后，进行计算 */
    return free_nodes;
}

/** 
 * __get_total_page_nr - 获取物理内存总页数
 * 
 * 返回物理内存总页数
 */
unsigned long __get_total_page_nr()
{
    return total_pmem_size / PAGE_SIZE;
}

/**
 * init_pmem - 初始化物理内存管理以及一些相关设定
 */
int init_pmem()
{
    //----获取内存大小----
    total_pmem_size = get_memory_size_from_hardware();
    /* 根据内存大小划分区域
    如果内存大于1GB:
        1G预留128MB给非连续内存，其余给内核和用户程序平分，内核多余的部分分给用户
    如果内存小于1GB：
        预留128MB给非连续内存，其余平分给内核和用户程序
     */
    unsigned int normal_size;
    unsigned int user_size;
    
    normal_size = (total_pmem_size - (NORMAL_MEM_ADDR + HIGH_MEM_SIZE + NULL_MEM_SIZE)) / 2; 
    user_size = total_pmem_size - normal_size;
    if (normal_size > 1*GB) {
        unsigned int more_size = normal_size - 1*GB;

        /* 挪出多余的给用户 */
        user_size += more_size;
        normal_size -= more_size;
    }
    /* 由于引导中只映射了0~8MB，所以这里从DMA开始 */
    mem_self_mapping(DMA_MEM_ADDR, NORMAL_MEM_ADDR + normal_size);
    
    /* 根据物理内存大小对内存分配器进行限定 */
    init_boot_mem(PAGE_OFFSET + NORMAL_MEM_ADDR , PAGE_OFFSET + (NORMAL_MEM_ADDR + normal_size));
    
    /* 节点数量就是页数量 */
    mem_node_count = (normal_size + user_size)/PAGE_SIZE;
    mem_node_base = NORMAL_MEM_ADDR;

    unsigned int mem_node_table_size = mem_node_count * SIZEOF_MEM_NODE;
    
    /* 分配内存节点数组 */
    mem_node_table = boot_mem_alloc(mem_node_table_size);
    if (mem_node_table == NULL) {
        panic("boot mem alloc for mem node table failed!\n");
    }
    
    //printk(KERN_DEBUG "mem node table at %x size:%x %d MB\n", (unsigned int )mem_node_table, mem_node_table_size, mem_node_table_size/MB);
    
    memset(mem_node_table, 0, mem_node_table_size);

    cut_used_mem();
#if 0   /* test */
    unsigned int a = __alloc_pages(1000);
    unsigned int b = __alloc_pages(2);
    unsigned int c = __alloc_pages(10);

    printk("a=%x,b=%x,c=%x\n", a, b, c);
    
    __free_pages(c);
    __free_pages(b);
    __free_pages(a);

    a = __alloc_pages(2);
    b = __alloc_pages(3);
    c = __alloc_pages(4);

    printk("a=%x,b=%x,c=%x\n", a, b, c);
    
    void *pa = __va((void *)a);
    void *pb = __pa(b);
    printk("a=%x,b=%x\n", pa, pb);

    memset(pa, 1, PAGE_SIZE * 2);
    
    __map_pages(0xc63fa000, 10 * PAGE_SIZE, PG_RW_W | PG_US_S);

    char *v = (char *)0xc63fa000;
    memset(v, 0xff, 10 * PAGE_SIZE);
    __unmap_pages(v, 10 * PAGE_SIZE);
    

    __map_pages_safe((void *)0xc53fa000, 10 * PAGE_SIZE, PG_RW_W | PG_US_S);

    v = (char *)0xc53fa000;
    memset(v, 0xff, 10 * PAGE_SIZE);
    __unmap_pages_safe(v, 10 * PAGE_SIZE);

    /*
    UnmapPages(0xc5000000, PAGE_SIZE * 100);

    MapPages(0xc6000000, PAGE_SIZE * 200, PAGE_RW_W | PAGE_US_S);

    v = (char *)0xc6000000;
    memset(v, 1, PAGE_SIZE * 200);

    UnmapPages(0xc6000000, PAGE_SIZE * 200);
	*/
    //memset(v, 0, PAGE_SIZE * 2);
#endif
    return 0;
}   
