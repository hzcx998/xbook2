#include <arch/phymem.h>
#include <arch/page.h>
#include <arch/bootmem.h>
#include <arch/memory.h>
#include <xbook/debug.h>
#include <math.h>
#include <string.h>

mem_node_t *mem_node_table;
unsigned int mem_node_count;
unsigned int mem_node_base;

static unsigned long total_pmem_size;

static mem_node_t *mem_alloc_node(unsigned int flags)
{
    mem_node_t *node = NULL;
    if (flags & MEM_NODE_TYPE_DMA)
        node = mem_node_table;
    else if (flags & MEM_NODE_TYPE_NORMAL)
        node = mem_node_table + (DMA_MEM_SIZE >> PAGE_SHIFT);
    else
        panic("phymem: get node null!");
    
    while (node < mem_node_table + mem_node_count) {
        if (!node->reference) {
            return node;
        }
        if (node->count) 
            node += node->count;
        else 
            node++;   
    }
    return NULL;
}

mem_node_t *phy_addr_to_mem_node(unsigned int page)
{ 
    int index = (page - mem_node_base) >> PAGE_SHIFT;
    mem_node_t *node = mem_node_table + index;
    if (node < mem_node_table || node >= mem_node_table + mem_node_count)
        return NULL;
    return node;
}

unsigned int mem_node_to_phy_addr(mem_node_t *node)
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




static void cut_used_mem()
{
    unsigned int used_mem = boot_mem_size();
    unsigned int used_pages = DIV_ROUND_UP(used_mem, PAGE_SIZE);
    page_alloc_normal(used_pages);
}

unsigned long mem_get_free_page_nr()
{
    mem_node_t *node = mem_node_table;
    unsigned long free_nodes = 0;
    while (node < mem_node_table + mem_node_count)
    {
        if (!node->reference) {
            free_nodes++;
        }
        if (node->count) 
            node += node->count;
        else 
            node++;
    }
    return free_nodes;
}

unsigned long mem_get_total_page_nr()
{
    return total_pmem_size / PAGE_SIZE;
}

unsigned long mem_node_alloc_pages(unsigned long count, unsigned long flags)
{
    mem_node_t *node = mem_alloc_node(flags);
    if (node == NULL)
        return 0;
    int index = node - mem_node_table;
    if (index + count > mem_node_count)
        return 0; 
    node->reference = 1;
    node->count = (unsigned int)count;
    node->flags = 0;
    node->cache = NULL;
    node->group = NULL;

    return (unsigned long)mem_node_to_phy_addr(node);
}

int mem_node_free_pages(unsigned long page)
{
    mem_node_t *node = phy_addr_to_mem_node((unsigned long)page);
    if (node == NULL)
        return -1;
    mem_atomic_dec(&node->reference);
	if (node->reference == 0) {
        node->count = 0;
		node->flags = 0;
        return 0;
	}
    return -1;
}

int physic_memory_init()
{
    total_pmem_size = phy_mem_get_size_from_hardware();
    /* 根据内存大小划分区域
    如果内存大于1GB:
        1G预留128MB给非连续内存，其余给内核和用户程序平分，内核多余的部分分给用户
    如果内存小于1GB：
        预留128MB给非连续内存，其余平分给内核和用户程序
     */
    unsigned int normal_size;
    unsigned int user_size;
    
    normal_size = (total_pmem_size - (NORMAL_MEM_ADDR + DYNAMIC_MAP_MEM_SIZE + KERN_BLACKHOLE_MEM_SIZE)) / 2; 
    user_size = total_pmem_size - normal_size;
    if (normal_size > 1*GB) {
        unsigned int more_size = normal_size - 1*GB;
        user_size += more_size;
        normal_size -= more_size;
    }
    
    /* 由于引导中只映射了0~8MB，所以这里从DMA开始 */
    kern_page_map_early(DMA_MEM_ADDR, NORMAL_MEM_ADDR + normal_size);
    boot_mem_init(KERN_BASE_VIR_ADDR + NORMAL_MEM_ADDR , KERN_BASE_VIR_ADDR + (NORMAL_MEM_ADDR + normal_size));
    
    #if 0
    mem_node_count = (normal_size + user_size)/PAGE_SIZE;
    mem_node_base = DMA_MEM_ADDR;

    unsigned int mem_node_table_size = mem_node_count * SIZEOF_MEM_NODE;
    mem_node_table = boot_mem_alloc(mem_node_table_size);
    if (mem_node_table == NULL) {
        panic("boot mem alloc for mem node table failed!\n");
    }
    memset(mem_node_table, 0, mem_node_table_size);
    #else
    mem_range_init(0, DMA_MEM_ADDR, NORMAL_MEM_ADDR);
    mem_range_init(1, NORMAL_MEM_ADDR, total_pmem_size - NORMAL_MEM_ADDR + PAGE_SIZE);
    
    mem_pool_test();
    #endif

    spin("test");
    cut_used_mem();
    return 0;
}   
