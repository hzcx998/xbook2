#include <arch/mempool.h>
#include <arch/page.h>
#include <arch/bootmem.h>
#include <xbook/debug.h>
#include <assert.h>
#include <math.h>
#include <stdint.h>

// #define MEMPOOL_DEBUG

mem_range_t mem_ranges[MEM_RANGE_NR];

void mem_node_init(mem_node_t *node, int ref, uint32_t size)
{
    node->reference = ref;
    node->count = size;
    node->cache = NULL;
    node->group = NULL;
    node->section = NULL;
    INIT_LIST_HEAD(&node->list);
}

void mem_section_init(mem_section_t *mem_section, size_t section_size)  
{   
    mem_section->node_count = 0;
    mem_section->section_size = section_size;
    INIT_LIST_HEAD(&mem_section->free_list_head);
}

void mem_section_setup(mem_section_t *mem_section, mem_node_t *node_base, size_t section_count)  
{    
    mem_section->node_count = section_count;
    int i;
    for (i = 0; i < section_count; i++) {
        mem_node_t *next = node_base + i * mem_section->section_size;
        mem_node_init(next, 0, 0);
        list_add_tail(&next->list, &mem_section->free_list_head);
        MEM_NODE_MARK_SECTION(next, mem_section);
    }
}

mem_range_t *mem_range_get_by_mem_node(mem_node_t *node)
{
    int i;
    mem_range_t *mem_range;
    for (i = 0; i < MEM_RANGE_NR; i++) {
        mem_range = &mem_ranges[i];
        if (mem_range->node_table <= node && node < mem_range->node_table + mem_range->pages) {
            return mem_range;
        } 
    }
    panic("mem node not in range!");
    return NULL;
}

mem_range_t *mem_range_get_by_phy_addr(unsigned int addr)
{
    int i;
    mem_range_t *mem_range;
    for (i = 0; i < MEM_RANGE_NR; i++) {
        mem_range = &mem_ranges[i];
        if (mem_range->start <= addr && addr < mem_range->end) {
            return mem_range;
        } 
    }
    panic("addr not in range!");
    return NULL;
}

void mem_range_init(unsigned int idx, unsigned int start, size_t len)
{
    if (idx >= MEM_RANGE_NR)
        return;
    mem_range_t *mem_range = &mem_ranges[idx];
    mem_range->start = start;
    mem_range->end = start + len;
    mem_range->pages = len / PAGE_SIZE;
    mem_range->node_table = boot_mem_alloc(mem_range->pages * sizeof(mem_node_t));
    int i;
    for (i = 0; i < MEM_SECTION_MAX_NR; i++) {    
        mem_section_init(&mem_range->sections[i], powi(2, i));
    }

    printk(KERN_INFO "mem range: start:%x end:%x pages:%d len:%dMB\n",
        mem_range->start, mem_range->end, mem_range->pages, len / MB);
    
    int section_off = MEM_SECTION_MAX_NR - 1;     
    size_t section_size = powi(2, section_off);
    mem_section_t *mem_section;
    size_t big_sections;
    size_t small_sections;
    
    mem_node_t *node_base = mem_range->node_table;
    ssize_t pages = mem_range->pages;

    while (pages > 0) {
        ASSERT(0 <= section_off && section_off < MEM_SECTION_MAX_NR);

        big_sections = pages / section_size;
        small_sections = pages % section_size;
        mem_section = &mem_range->sections[section_off];
        
        if (big_sections > 0) {
            mem_section_setup(mem_section, node_base, big_sections);
            pages -= big_sections * section_size;
            node_base += big_sections * section_size;
        } else if (small_sections > 0) {
            /* 把剩下的所有小的节都放到大小为1的节中。 */
            mem_section = &mem_range->sections[0];
            mem_section_setup(mem_section, node_base, small_sections);
            break;
        }
        
        section_size >>= 1;
        --section_off;
    }
}

mem_node_t *phy_addr_to_mem_node(unsigned int addr)
{ 
    mem_range_t *mem_range = mem_range_get_by_phy_addr(addr);
    if (!mem_range)
        return NULL;
    unsigned int local_addr = addr - mem_range->start;
    unsigned int index = local_addr >> PAGE_SHIFT;
    mem_node_t *node = mem_range->node_table + index;
    return node;
}

unsigned int mem_node_to_phy_addr(mem_node_t *node)
{ 
    mem_range_t *mem_range = mem_range_get_by_mem_node(node);
    if (!mem_range)
        return 0;
    unsigned int index =  node - mem_range->node_table;
    unsigned int local_addr = index << PAGE_SHIFT;
    return local_addr + mem_range->start; 
}

int mem_range_split_section(mem_range_t *mem_range, mem_section_t *mem_section)
{
    /* 已经有节有空闲的节点，成功返回！ */
    if (!list_empty(&mem_section->free_list_head))
        return 0;

    mem_section_t *tmp_section = mem_section + 1;
    mem_section_t *top_section = &mem_range->sections[MEM_SECTION_MAX_NR - 1];
    while (tmp_section <= top_section) {
        if (!list_empty(&tmp_section->free_list_head))
            break;
        tmp_section++;
    }
    
    if (tmp_section > top_section) {
        // TODO: 收缩内存
        printk(KERN_ERR "mempool: no free section left!\n");
        return -1;
    }

    mem_node_t *node = list_first_owner(&tmp_section->free_list_head, mem_node_t, list);
    list_del(&node->list);
    MEM_SECTION_DES_COUNT(tmp_section);

    mem_node_init(node, 1, tmp_section->section_size / 2);
    
    mem_node_t *node_half = node + node->count;
    mem_node_init(node_half, 1, node->count);

    --tmp_section; // 下降一个节高度
    list_add(&node->list, &tmp_section->free_list_head);
    list_add(&node_half->list, &tmp_section->free_list_head);
    MEM_NODE_MARK_SECTION(node, tmp_section);
    MEM_NODE_MARK_SECTION(node_half, tmp_section);
    MEM_SECTION_INC_COUNT(tmp_section);
    MEM_SECTION_INC_COUNT(tmp_section);

    return mem_range_split_section(mem_range, mem_section);
}

unsigned long mem_node_alloc_pages(unsigned long count, unsigned long flags)
{
    if (count > MEM_SECTION_MAX_SIZE) {
        printk(KERN_NOTICE "%s: page count %d too big!\n", __func__, count);
        return 0;
    }
    mem_range_t *mem_range = NULL;

    if (flags & MEM_NODE_TYPE_DMA)
        mem_range = &mem_ranges[MEM_RANGE_DMA];
    else if (flags & MEM_NODE_TYPE_NORMAL)
        mem_range = &mem_ranges[MEM_RANGE_NORMAL];
    else if (flags & MEM_NODE_TYPE_USER)
        mem_range = &mem_ranges[MEM_RANGE_USER];
    else
        panic("phymem: get range null!");
    
    mem_section_t *mem_section;
    int i;
    for (i = 0; i < MEM_SECTION_MAX_NR; i++) {
        mem_section = &mem_range->sections[i];
        if (mem_section->section_size >= count) {
            break;
        }
    }
    if (list_empty(&mem_section->free_list_head)) {
        if (mem_section->section_size == MEM_SECTION_MAX_SIZE) {    // 没有更大的节
            // TODO: 收缩内存，合并没有使用的小节为大节
            printk(KERN_ERR "mempool: no free section!\n");
            return 0;
        } else {
            if (mem_range_split_section(mem_range, mem_section) < 0) {
                printk(KERN_ERR "mempool: split section failed!\n");
                return 0;
            }
        }
    }
    mem_node_t *node = list_first_owner(&mem_section->free_list_head, mem_node_t, list);
    list_del(&node->list);
    MEM_SECTION_DES_COUNT(mem_section);

    mem_node_init(node, 1, count);
    MEM_NODE_MARK_SECTION(node, mem_section);

    return mem_node_to_phy_addr(node);
}

int mem_node_free_pages(unsigned long addr)
{
    mem_node_t *node = phy_addr_to_mem_node(addr);
    if (!node)
        return -1;
    
    // 放回节点所属的节中去
    mem_section_t *section = MEM_NODE_GET_SECTION(node);
    if (!section) {
        printk(KERN_WARING "node no section!\n");
        return -1;
    }
    if (list_find(&node->list, &section->free_list_head)) {
        printk(KERN_WARING "addr %x don't need free again!\n", addr);
        return -1;
    }
    mem_node_init(node, 0, 0);
    list_add(&node->list, &section->free_list_head);
    MEM_SECTION_INC_COUNT(section);

    return 0;
}

unsigned long mem_get_free_page_nr()
{
    size_t page_count = 0;
    int i, j;
    for (j = 0; j < MEM_RANGE_NR; j++) {
        mem_range_t *range = &mem_ranges[j];
        for (i = 0; i < MEM_SECTION_MAX_NR; i++) {
            mem_section_t *section = &range->sections[i];
            page_count += section->node_count * section->section_size;
        }
    }
    return page_count;
}

void mem_pool_test()
{
    uint32_t addr = 64 * MB;
    mem_node_t *node = phy_addr_to_mem_node(addr);
    printk("addr mem node: %x\n", node);
    printk("new addr: %x\n", mem_node_to_phy_addr(node));

    #if 1
    int i;
    for (i = 0; i < MEM_SECTION_MAX_NR; i++) {    
        addr = mem_node_alloc_pages(powi(2, i), MEM_NODE_TYPE_NORMAL);
        printk("alloc addr: %x\n", addr);
        if (!addr)
            break;
        mem_node_free_pages(addr);
    }
    for (i = 0; i <= MEM_SECTION_MAX_NR; i++) {    
        addr = mem_node_alloc_pages(powi(2, i), MEM_NODE_TYPE_DMA);
        printk("2 alloc addr: %x\n", addr);
        if (!addr)
            break;
        //mem_node_free_pages(addr);
    }
    #endif
    spin("test");
}
