#include <arch/mempool.h>
#include <arch/page.h>

mem_range_t mem_ranges[MEM_RANGE_NR];

void mem_range_init(unsigned int idx, unsigned int start, unsigned int end)
{
    if (idx >= MEM_RANGE_NR)
        return;
    mem_range_t *mrange = &mem_ranges[idx];
    mrange->start = start;
    mrange->end = end;
    mrange->pages = (end - start) / PAGE_SIZE;

    ssize_t pages = mrange->pages;
    size_t section_size = MEM_SECTION_MAX_SIZE;
    
    size_t big_sections;
    size_t small_sections;
    
    while (pages > 0) {    
        big_sections = pages / section_size;
        small_sections = pages % section_size;
        
        if (big_sections > 0) {
            // big_sections * section_size
            printk("big section: %d * %d\n", big_sections, section_size);
            pages -= big_sections * section_size;
        } else if (small_sections > 0) {
            // small_sections * 1
            printk("small section: %d\n", small_sections);
            pages -= small_sections;
            //break;
        }
        section_size >>= 1;
    } 
}
