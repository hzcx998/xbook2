/*
 * file:		kernel/mm/mem_cache.c
 * auther:		Jason Hu
 * time:		2019/10/1
 * copyright:	(C) 2018-2020 by Book OS developers. All rights reserved.
 */

#include <arch/page.h>
#include <arch/interrupt.h>
#include <arch/phymem.h>
#include <xbook/config.h>
#include <xbook/memcache.h>
#include <xbook/debug.h>
#include <string.h>
#include <math.h>
#include <string.h>
#include <xbook/bitmap.h>
#include <xbook/vmm.h>

static cache_size_t cache_size[] = {
	#if PAGE_SIZE == 4096
	{32, NULL},
	#endif
	{64, NULL},
	{128, NULL},
	{256, NULL},
	{512, NULL},
	{1024, NULL},
	{2048, NULL},
	{4096, NULL},
	{8*1024, NULL},
	{16*1024, NULL},
	{32*1024, NULL},
	{64*1024, NULL},
	{128*1024, NULL},	
	/* 配置大内存的分配 */
	#ifdef CONFIG_LARGE_ALLOCS
	{256*1024, NULL},
	{512*1024, NULL},
	{1024*1024, NULL},
	{2*1024*1024, NULL},
	#endif
	{0, NULL},
};

#if PAGE_SIZE == 4096
	#ifdef CONFIG_LARGE_ALLOCS 
		#define MAX_MEM_CACHE_NR 17
	#else
		#define MAX_MEM_CACHE_NR 13
	#endif
#else
	#ifdef CONFIG_LARGE_ALLOCS 
		#define MAX_MEM_CACHE_NR 16
	#else
		#define MAX_MEM_CACHE_NR 12
	#endif
#endif

mem_cache_t mem_caches[MAX_MEM_CACHE_NR];
LIST_HEAD(large_mem_object_list);
DEFINE_MUTEX_LOCK(large_mem_mutex);

void mem_cache_dump(mem_cache_t *cache)
{
	keprint("----Mem Cache----\n");
	keprint("object size %d count %d\n", cache->object_size, cache->object_count);
	keprint("flags %x name %s\n", cache->flags, cache->name);
	keprint("full %x partial %x free %x\n", cache->full_groups, cache->partial_groups, cache->free_groups);
}

void mem_group_dump(mem_group_t *group)
{
	keprint("----Mem Group----\n");
	keprint("map bits %x len %d\n", group->map.bits, group->map.byte_length);
	keprint("objects %x flags %x list %x\n", group->objects, group->flags, group->list);
	keprint("using %d free %x\n", group->using_count, group->free_count);
}

int mem_cache_init(mem_cache_t *cache, char *name, size_t size, flags_t flags)
{
	if (!size)
		return -1;
	list_init(&cache->full_groups);
	list_init(&cache->partial_groups);
	list_init(&cache->free_groups);

	if (size < 1024) {
		unsigned int group_size = ALIGN_WITH(SIZEOF_MEM_GROUP, 8);
		unsigned int left_size = PAGE_SIZE - group_size - 16;
		cache->object_count = left_size / size;;
	} else if (size <= 128 * 1024) {
		cache->object_count = (1 * MB) / size;
	} else if (size <= 4 * 1024 * 1024) {
		cache->object_count = (4 * MB) / size;
	} else {
        /* 超过最大范围，则每一个缓存4个对象 */
        cache->object_count = 4;
    }

	cache->object_size = size;
	cache->flags = flags;
	memset(cache->name, 0, MEM_CACHE_NAME_LEN);
	strcpy(cache->name, name);
    mutexlock_init(&cache->mutex);
	return 0;
}

static void *mem_cache_page_alloc(unsigned long count)
{
	unsigned long page = page_alloc_normal(count);
	if (!page)
		return NULL;
	return kern_phy_addr2vir_addr(page);
}

static int mem_cache_page_free(void *address)
{
	if (address == NULL)
		return -1;
	unsigned int page = kern_vir_addr2phy_addr(address);
	if (!page)
		return -1;
	page_free(page);
	return 0;
}

static int mem_group_init(
    mem_cache_t *cache,
	mem_group_t *group,
	flags_t flags)
{
	list_add(&group->list, &cache->free_groups);
	unsigned char *map = (unsigned char *)(group + 1);
	group->map.byte_length = DIV_ROUND_UP(cache->object_count, 8);
	group->map.bits = (unsigned char *)map;	
	bitmap_init(&group->map);
	mem_node_t *node; 
	if (cache->object_size < 1024) {
		group->objects = map + 16;
		node = phy_addr_to_mem_node(kern_vir_addr2phy_addr(group));
		CHECK_MEM_NODE(node);
		MEM_NODE_MARK_CHACHE_GROUP(node, cache, group);
	} else {
		unsigned int pages = DIV_ROUND_UP(cache->object_count * cache->object_size, PAGE_SIZE); 
		group->objects = mem_cache_page_alloc(pages);
		if (group->objects == NULL) {
			keprint(PRINT_ERR "alloc page for mem objects failed\n");
			return -1;
		}
		int i;
		for (i = 0; i < pages; i++) {
			node = phy_addr_to_mem_node(kern_vir_addr2phy_addr(group->objects + i * PAGE_SIZE));
			CHECK_MEM_NODE(node);
			MEM_NODE_MARK_CHACHE_GROUP(node, cache, group);
		}
	}

	group->using_count = 0;
	group->free_count = cache->object_count;
	group->flags =  flags;
	return 0;
}

static int mem_group_create(mem_cache_t *cache, flags_t flags)
{
	mem_group_t *group;
	group = mem_cache_page_alloc(1);
	if (group == NULL) {
		keprint(PRINT_ERR "alloc page for mem group failed!\n");
		return -1;
	}
	if (mem_group_init(cache, group, flags)) {
		keprint(PRINT_ERR "init mem group failed!\n");
		goto free_group;
	}
	return 0;
free_group:
	mem_cache_page_free(group);
	return -1;
}

static int mem_caches_build()
{
	cache_size_t *cachesz = cache_size;
	mem_cache_t *mem_cache = &mem_caches[0];
	while (cachesz->cache_size) {
		if (mem_cache_init(mem_cache, "mem cache", cachesz->cache_size, 0)) {
			keprint("create mem cache failed!\n");
			return -1;
		}
		cachesz->mem_cache = mem_cache;
		mem_cache++;
		cachesz++;
	}
	return 0;
}

static void *mem_cache_do_alloc(mem_cache_t *cache, mem_group_t *group)
{
	void *object;
	int idx = bitmap_scan(&group->map, 1);
	if (idx == -1) {
		keprint(PRINT_EMERG "bitmap scan failed!\n");
		return NULL;
	}
	bitmap_set(&group->map, idx, 1);
	object = group->objects + idx * cache->object_size;
	group->using_count++;
	group->free_count--;
	if (group->free_count == 0) {
		list_del(&group->list);
		list_add_tail(&group->list, &cache->full_groups);
	}
	return object;
}

void *mem_cache_alloc_object(mem_cache_t *cache)
{
	void *object;
	mem_group_t *group;
	list_t *partialList, *node;
retry_alloc_object:
    mutex_lock(&cache->mutex);
	partialList = &cache->partial_groups;
	node = partialList->next;
	if (list_empty(partialList)) {
		list_t *freeList;
		freeList = &cache->free_groups;
		if (list_empty(freeList)) {
			goto new_group;
		}
		node = freeList->next;
		list_del(node);
		list_add_tail(node, partialList);
	}
	group = list_owner(node, mem_group_t, list);
	object = mem_cache_do_alloc(cache, group);
    mutex_unlock(&cache->mutex);
	return object;
new_group:
    mutex_unlock(&cache->mutex);
	if (mem_group_create(cache, 0))
		return NULL;
	goto retry_alloc_object;
	return NULL;
}

void *mem_alloc(size_t size)
{
	if (size > MAX_MEM_CACHE_SIZE) {
        if (size > MAX_MEM_OBJECT_SIZE)
            return NULL;
        /* 使用首适配分配法 */
        large_mem_object_t *obj = mem_alloc(sizeof(large_mem_object_t));
        if (obj == NULL)
            return NULL;
        obj->size = size;
        obj->addr = mem_cache_page_alloc(DIV_ROUND_UP(size, PAGE_SIZE));
        if (obj->addr == NULL) {
            mem_free(obj);
            return NULL;
        }
        mutex_lock(&large_mem_mutex);
        list_add(&obj->list, &large_mem_object_list);
        mutex_unlock(&large_mem_mutex);
        //keprint(PRINT_DEBUG "[memcache]: alloc large mem object %x\n", obj->addr);
		return obj->addr;
	}
	cache_size_t *cachesz = &cache_size[0];
	while (cachesz->cache_size) {
		if (cachesz->cache_size >= size)
			break;
		cachesz++;
	}
    void *p = mem_cache_alloc_object(cachesz->mem_cache);
    return p;
}

void *mem_alloc_align(size_t size, int align)
{
	void *p = mem_alloc(size);
    if (p == NULL)
        return p;
    void *q = (void *)((unsigned long)p & (~(align - 1)));
    return q;
}

void *mem_zalloc(size_t size)
{
    void *ret = mem_alloc(size);

    if (!ret)
    {
        return NULL;
    }

    return memset(ret, 0, size);
}

void *mem_realloc(void *ptr, size_t size)
{
    void *ret = mem_alloc(size);

    if (!ret)
    {
        return NULL;
    }

    if (ptr)
    {
        memcpy((char*)ret, (char*)ptr, size);
        mem_free(ptr);
    }
    return ret;
}

static void mem_cache_do_free(mem_cache_t *cache, void *object)
{
	mem_group_t *group;
	mem_node_t *node = phy_addr_to_mem_node(kern_vir_addr2phy_addr(object));

	CHECK_MEM_NODE(node);
	group = MEM_NODE_GET_GROUP(node);
	if (group == NULL) 
		panic(PRINT_EMERG "group get from page bad!\n");
	int index = (((unsigned char *)object) - group->objects)/cache->object_size; 
	if (index < 0 || index > group->map.byte_length*8)
		panic(PRINT_EMERG "map index bad range!\n");
    mutex_lock(&cache->mutex);
	bitmap_set(&group->map, index, 0);
	int unsing = group->using_count;
	group->using_count--;
	group->free_count++;
	
	if (!group->using_count) {
		list_del(&group->list);
		list_add_tail(&group->list, &cache->free_groups);
	} else if (unsing == cache->object_count) {
		list_del(&group->list);
		list_add_tail(&group->list, &cache->partial_groups);
	}
    mutex_unlock(&cache->mutex);
}

void mem_cache_free_object(mem_cache_t *cache, void *object)
{
	mem_cache_do_free(cache, object);
}

void mem_free(void *object)
{
	if (!object)
		return;
	mem_cache_t *cache;
	mem_node_t *node = phy_addr_to_mem_node(kern_vir_addr2phy_addr(object));
	CHECK_MEM_NODE(node);
	cache = MEM_NODE_GET_CACHE(node);
    if (cache == NULL) {
        /* 采用首适配释放 */
        mutex_lock(&large_mem_mutex);
        large_mem_object_t *obj;
        list_for_each_owner (obj, &large_mem_object_list, list) {
            if (obj->addr == object) {
                list_del(&obj->list);
                mem_cache_page_free(obj->addr);
                mem_free(obj);
                mutex_unlock(&large_mem_mutex);
                // keprint(PRINT_DEBUG "[memcache]: free large mem object %x\n", object);
                return;
            }
        }
        mutex_unlock(&large_mem_mutex);
        return;
    }
	mem_cache_free_object(cache, (void *)object);
}

static int group_destory(mem_cache_t *cache, mem_group_t *group)
{
	list_del(&group->list);
	if (cache->object_size < 1024) {
		if (mem_cache_page_free(group))
			return -1;
	} else {
		if (mem_cache_page_free(group->objects))
			return -1;
		if (mem_cache_page_free(group))
			return -1;
	}
	return 0;
}

static int mem_cache_do_shrink(mem_cache_t *cache)
{
	mem_group_t *group, *next;
	int ret = 0;
	list_for_each_owner_safe(group, next, &cache->free_groups, list) {
		if(!group_destory(cache, group))
			ret++;
	}
	return ret;
}

static int mem_cahce_shrink(mem_cache_t *cache)
{
	int ret;
	if (!cache) 
		return 0; 
	unsigned long flags;
    interrupt_save_and_disable(flags);
	ret = mem_cache_do_shrink(cache);
    interrupt_restore_state(flags);
	return ret * cache->object_count * cache->object_size;
}

int mem_shrink()
{
	size_t size = 0;
	cache_size_t *cachesz = &cache_size[0];
	while (cachesz->cache_size) {
		size += mem_cahce_shrink(cachesz->mem_cache);
		cachesz++;
	}
	return size;
}

int mem_caches_init()
{
	mem_caches_build();
    infoprint("vmm: user base: %x, size: %x, top: %x, stack top:%x\n", 
        USER_VMM_BASE_ADDR, USER_VMM_SIZE, USER_VMM_TOP_ADDR, USER_STACK_TOP);
	return 0;
}
