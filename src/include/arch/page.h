#ifndef _ARCH_PAGE_H
#define _ARCH_PAGE_H

#include "general.h"


static inline unsigned long v2p(void *address)
{
    return __pa(address);
}

static inline void *p2v(unsigned long address)
{
    return __va(address);
}

#define alloc_pages(count)                  __alloc_pages(count)
#define free_pages(addr)                    __free_pages(addr)

#define alloc_page()                          alloc_pages(1)
#define free_page(addr)                     free_pages(addr)

#define page_link(va, pa, prot)             __page_link(va, pa, prot)
#define page_unlink(va)                     __page_unlink(va)

#define map_pages(start, len, prot)         __map_pages(start, len, prot)
#define unmap_pages(va, len)                __unmap_pages(va, len)

#define map_pages_safe(start, len, prot)    __map_pages_safe(start, len, prot);
#define unmap_pages_safe(start, len)        __unmap_pages_safe(start, len);

#define addr2page(addr)                     __page2mem_node(addr);
#define page2addr(page)                     __mem_node2page(page);

#define get_free_page_nr                    __get_free_page_nr
#define get_total_page_nr                   __get_total_page_nr

#endif  /* _ARCH_PAGE_H */
