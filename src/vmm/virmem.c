#include <arch/interrupt.h>
#include <arch/memio.h>
#include <xbook/virmem.h>
#include <xbook/debug.h>
#include <xbook/bitmap.h>
#include <xbook/memcache.h>
#include <string.h>

static bitmap_t vir_addr_bitmap;
static unsigned long vir_addr_base; 
static list_t using_vir_mem_list;
static list_t free_vir_mem_list;

unsigned long vir_addr_alloc(size_t size)
{
	size = PAGE_ALIGN(size);
	if (!size)
		return 0;
	long pages = size / PAGE_SIZE;
	long idx = bitmap_scan(&vir_addr_bitmap, pages);
	if (idx == -1)
		return 0;
	int i;
	for (i = 0; i < pages; i++) {
		bitmap_set(&vir_addr_bitmap, idx + i, 1);
	}
	return vir_addr_base + idx * PAGE_SIZE; 
}

unsigned long vir_addr_free(unsigned long vaddr, size_t size)
{
    if (!size)
        return -1;
    size = PAGE_ALIGN(size);
	long pages = size / PAGE_SIZE;
	long idx = (vaddr - vir_addr_base) / PAGE_SIZE;
	if (idx == -1)
		return -1;
	long i;
	for (i = 0; i < pages; i++) {
		bitmap_set(&vir_addr_bitmap, idx + i, 0);
	}
	return 0; 
}

static void *vir_mem_do_alloc(size_t size)
{
	unsigned long start = vir_addr_alloc(size);
	if (!start) 
		return NULL;
	
	vir_mem_t *area;
	area = mem_alloc(sizeof(vir_mem_t));
	if (area == NULL) {
		vir_addr_free(start, size);
		return NULL;
	}
	area->addr = start;
	area->size = size;

	unsigned long flags;
    interrupt_save_and_disable(flags);
	list_add_tail(&area->list, &using_vir_mem_list);
	if (page_map_addr(start, size, PROT_KERN | PROT_WRITE)) {
		vir_addr_free(start, size);
		mem_free(area);
		interrupt_restore_state(flags);
		return NULL;
	}
	interrupt_restore_state(flags);
	return (void *)area->addr;
}

void *vir_mem_alloc(size_t size)
{
	size = PAGE_ALIGN(size);

	if (!size)
		return NULL;

	size += PAGE_SIZE;
	vir_mem_t *target = NULL, *area;
	list_for_each_owner(area, &free_vir_mem_list, list) {
		if (size >= area->size) {
			target = area;
			break;
		}
	}
	if (target != NULL) {
		unsigned long flags;
        interrupt_save_and_disable(flags);
		list_del(&target->list);
		list_add_tail(&target->list, &using_vir_mem_list);
		interrupt_restore_state(flags);
		return (void *)target->addr;
	}
	return (void *)vir_mem_do_alloc(size);
}

static int vir_mem_do_free(vir_mem_t *target)
{
	vir_mem_t *area;
	list_del(&target->list);
	char inserted = 0;
	if (list_empty(&free_vir_mem_list)) {
		list_add(&target->list, &free_vir_mem_list);
	} else {
		area = list_first_owner(&free_vir_mem_list, vir_mem_t, list);
		do {
			if (target->size > area->size) {
				if (area->list.next == &free_vir_mem_list) {
					list_add_tail(&target->list, &free_vir_mem_list);
					inserted = 1;
					break;
				}
				area = list_owner(area->list.next, vir_mem_t, list);
			} else {
				list_add_before(&target->list, &area->list);
				inserted = 1;
				break;
			}
		} while (&area->list != &free_vir_mem_list);
	}
	return inserted;
}

int vir_mem_free(void *ptr)
{
	if (ptr == NULL)
		return -1;
	unsigned long addr = (unsigned long)ptr;
	if (addr < vir_addr_base || addr >= VIR_MEM_END)
		return -1;
	vir_mem_t *target = NULL, *area;
	unsigned long flags;
    interrupt_save_and_disable(flags);
	list_for_each_owner(area, &using_vir_mem_list, list) {
		if (area->addr == addr) {
			target = area;
			break;
		}
	}
	if (target != NULL) {
		if (vir_mem_do_free(target)) {
			interrupt_restore_state(flags);
			return 0;
		}
	}
    interrupt_restore_state(flags);
	return -1;
}

void *memio_remap(unsigned long paddr, size_t size)
{
    if (!paddr || !size) {
        return NULL;
    }
    unsigned long vaddr = vir_addr_alloc(size);
    if (vaddr == -1) {
        printk("alloc virtual addr for IO remap failed!\n");
        return NULL;
    }
	vir_mem_t *area;
	area = mem_alloc(sizeof(vir_mem_t));
	if (area == NULL) {
		vir_addr_free(vaddr, size);
		return NULL;
	}
	area->addr = vaddr;
	area->size = size;
    unsigned long flags;
    interrupt_save_and_disable(flags);
	list_add_tail(&area->list, &using_vir_mem_list);
    if (mem_remap(paddr, vaddr, size)) {
        list_del(&area->list);
        mem_free(area);
        vir_addr_free(vaddr, size);
        vaddr = 0;
    }
    interrupt_restore_state(flags);
    return (void *)vaddr;    
}

int memio_unmap(void *vaddr)
{
    if (vaddr == NULL) {
        return -1;
    }
    unsigned long addr = (unsigned long )vaddr;
	if (addr < vir_addr_base || addr >= VIR_MEM_END)
		return -1;
	vir_mem_t *target = NULL, *area;
	unsigned long flags;
    interrupt_save_and_disable(flags);
	list_for_each_owner(area, &using_vir_mem_list, list) {
		if (area->addr == addr) {
			target = area;
			break;
		}
	}
	if (target != NULL) {
        if (mem_unmap(target->addr, target->size)) {
            list_del(&target->list);
            vir_addr_free(addr, target->size);
            mem_free(target);
            interrupt_restore_state(flags);
            return 0;
		}
	}
	interrupt_restore_state(flags);
    return -1;
}

void vir_mem_init()
{
	vir_addr_bitmap.byte_length = DYNAMIC_MAP_MEM_SIZE / (PAGE_SIZE * 8);
	vir_addr_bitmap.bits = mem_alloc(vir_addr_bitmap.byte_length);
	bitmap_init(&vir_addr_bitmap);
	vir_addr_base = VIR_MEM_BASE;
	INIT_LIST_HEAD(&using_vir_mem_list);
	INIT_LIST_HEAD(&free_vir_mem_list);
}
