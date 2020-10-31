#include <arch/mempool.h>
#include <arch/page.h>
#include <arch/bootmem.h>
#include <xbook/debug.h>
#include <assert.h>
#include <math.h>
#include <stdint.h>

mem_range_t mem_ranges[MEM_RANGE_NR];

void mem_section_init(mem_section_t *mem_section, mem_node_t *node_base, size_t section_size, size_t section_count)  
{    
    // 创建节头
    mem_section->node_count = section_count;
    INIT_LIST_HEAD(&mem_section->node_list_head);
    int i;
    for (i = 0; i < section_count; i++) {
        mem_node_t *next_one = node_base + i * section_size;
        list_add_tail(&next_one->list, &mem_section->node_list_head);
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

void mem_range_init(unsigned int idx, unsigned int start, unsigned int end)
{
    if (idx >= MEM_RANGE_NR)
        return;
    mem_range_t *mem_range = &mem_ranges[idx];
    mem_range->start = start;
    mem_range->end = end;
    mem_range->pages = (end - start) / PAGE_SIZE;
    mem_range->node_table = boot_mem_alloc(mem_range->pages * sizeof(mem_node_t));
    int i;
    for (i = 0; i < MEM_SECTION_MAX_NR; i++) {    
        mem_range->sections[i].node_count = 0;
        INIT_LIST_HEAD(&mem_range->sections[i].node_list_head);
    }
    printk(KERN_NOTICE "mem range: start:%x end:%x pages:%d node table:%x\n",
        mem_range->start, mem_range->end, mem_range->pages, mem_range->node_table);
    
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
            mem_section_init(mem_section, node_base, section_size, big_sections);
            pages -= big_sections * section_size;
            node_base += big_sections * section_size;
        } else if (small_sections > 0) {
            /* 把剩下的所有小的节都放到大小为1的节中。 */
            mem_section = &mem_range->sections[0];
            printk("small section: offset %d node size %d, count %d\n", 0, 1, small_sections);
            mem_section_init(mem_section, node_base, 1, small_sections);
            break;
        }
        
        section_size >>= 1;
        --section_off;
    }
    printk(KERN_NOTICE "range table:%x-%x \n", mem_range->node_table, mem_range->node_table + mem_range->pages);

    mem_range = &mem_ranges[idx];

    mem_node_t *node;
    mem_range_t *tmp_range;
    for (i = 0; i < MEM_SECTION_MAX_NR; i++) {  
        list_for_each_owner (node, &mem_range->sections[i].node_list_head, list) {
            // printk("node:%x ", node);
            tmp_range = mem_range_get_by_mem_node(node);
            if (tmp_range != NULL) {
                // printk("range:%x - %x\n", tmp_range->start, tmp_range->end);
            } else {
                printk("range: null\n");
            }
        }
    }
}

mem_node_t *phy_addr_to_mem_node2(unsigned int addr)
{ 
    mem_range_t *mem_range = mem_range_get_by_phy_addr(addr);
    if (!mem_range)
        return NULL;
    unsigned int local_addr = addr - mem_range->start;
    unsigned int index = local_addr >> PAGE_SHIFT;
    mem_node_t *node = mem_range->node_table + index;
    return node;
}

unsigned int mem_node_to_phy_addr2(mem_node_t *node)
{ 
    mem_range_t *mem_range = mem_range_get_by_mem_node(node);
    if (!mem_range)
        return 0;
    
    unsigned int index =  node - mem_range->node_table;
    unsigned int local_addr = index << PAGE_SHIFT;
    return local_addr + mem_range->start; 
}

void mem_pool_test()
{
    uint32_t addr = 64 * MB;
    mem_node_t *node = phy_addr_to_mem_node2(addr);
    printk("addr mem node: %x\n", node);
    printk("new addr: %x\n", mem_node_to_phy_addr2(node));
}
