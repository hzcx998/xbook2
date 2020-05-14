/*
 * file:		kernel/mm/mem_cache.c
 * auther:		Jason Hu
 * time:		2019/10/1
 * copyright:	(C) 2018-2020 by Book OS developers. All rights reserved.
 */

#include <arch/page.h>
#include <arch/interrupt.h>
#include <xbook/config.h>
#include <xbook/memcache.h>
#include <xbook/debug.h>
#include <xbook/string.h>
#include <xbook/math.h>
#include <xbook/memops.h>
#include <xbook/bitmap.h>

/*
 * cache_size - cache大小的描述
 */
static cache_size_t cache_size[] = {
	#if PAGE_SIZE == 4096
	{32, NULL},
	#endif
	{64, NULL},
	{128, NULL},
	{256, NULL},
	/* 小于1KB的，就在单个页中进行分割 */
	{512, NULL},
	{1024, NULL},
	{2048, NULL},
	{4096, NULL},
	{8*1024, NULL},
	{16*1024, NULL},
	{32*1024, NULL},
	{64*1024, NULL},
	/* 通常情况下最大支持128KB的内存分配，需要在1MB内完成对象分割 */
	{128*1024, NULL},	
	/* 配置大内存的分配 */
	#ifdef CONFIG_LARGE_ALLOCS
	{256*1024, NULL},
	{512*1024, NULL},
	{1024*1024, NULL},
	{2*1024*1024, NULL},
	{4*1024*1024, NULL},
	#endif
	{0, NULL},		// 用于索引判断结束
};

/*
 * 通过页的大小来选择cache数量
 */
#if PAGE_SIZE == 4096
	#ifdef CONFIG_LARGE_ALLOCS 
		#define MAX_MEM_CACHE_NR 18
	#else
		#define MAX_MEM_CACHE_NR 13
	#endif
#else
	#ifdef CONFIG_LARGE_ALLOCS 
		#define MAX_MEM_CACHE_NR 17
	#else
		#define MAX_MEM_CACHE_NR 12
	#endif
#endif

/* 最开始的groupcache */
mem_cache_t mem_cache_table[MAX_MEM_CACHE_NR];

void dump_mem_cache(mem_cache_t *cache)
{
	printk("----Mem Cache----\n");
	printk("object size %d count %d\n", cache->object_size, cache->object_count);
	printk("flags %x name %s\n", cache->flags, cache->name);
	printk("full %x partial %x free %x\n", cache->full_groups, cache->partial_groups, cache->free_groups);
}

void dump_mem_group(mem_group_t *group)
{
	printk("----Mem Group----\n");
	printk("map bits %x len %d\n", group->map.bits, group->map.byte_length);
	printk("objects %x flags %x list %x\n", group->objects, group->flags, group->list);
	printk("using %d free %x\n", group->using_count, group->free_count);
}


int mem_cache_init(mem_cache_t *cache, char *name, size_t size, flags_t flags)
{
	if (!size)
		return -1;
	// 初始化链表
	INIT_LIST_HEAD(&cache->full_groups);
	INIT_LIST_HEAD(&cache->partial_groups);
	INIT_LIST_HEAD(&cache->free_groups);

	/* 根据size来选择不同的储存方式，以节约内存 */
	if (size < 1024) { // 如果是小于1024，那么就放到单个页中。
		/* 地址和8字节对齐，因为在结构体后面存放位图，位图是8字节为单位的 */
		unsigned int group_size = ALIGN_WITH(SIZEOF_MEM_GROUP, 8);

		/* 如果是32字节为准，那么，就位图就只需要16字节即可 */
		unsigned int left_size = PAGE_SIZE - group_size - 16;

		// 对象数量
		cache->object_count = left_size / size;;
	} else if (size <= 128 * 1024) {  // 如果是小于128kb，就放到1MB以内
		cache->object_count = (1 * MB) / size;
	} else if (size <= 4 * 1024 * 1024) { // 如果是小于4MB，就放到4MB以内
		cache->object_count = (4 * MB) / size;
	} else {
        /* 超过最大范围，则每一个缓存4个对象 */
        cache->object_count = 4;
    }

	// 对象的大小
	cache->object_size = size;

	// 设定cache的标志
	cache->flags = flags;
	
	// 设置名字
	memset(cache->name, 0, MEM_CACHE_NAME_LEN);
	strcpy(cache->name, name);

	//dump_mem_cache(cache);
	return 0;
}

static void *mem_cache_alloc_pages(unsigned long count)
{
	unsigned long page = alloc_pages(count);
	if (!page)
		return NULL;
	
	return p2v(page);
}

static int mem_cache_free_pages(void *address)
{
	if (address == NULL)
		return -1;

	unsigned int page = v2p(address);
	
	if (!page)
		return -1;
	
	free_pages(page);
	return 0;
}

static int mem_group_init(
    mem_cache_t *cache,
	mem_group_t *group,
	flags_t flags
) {
	// 把group添加到free链表
	list_add(&group->list, &cache->free_groups);

	// 位图位于group结构后面
	unsigned char *map = (unsigned char *)(group + 1);

	// 设定位图
	group->map.byte_length = DIV_ROUND_UP(cache->object_count, 8);
	group->map.bits = (unsigned char *)map;	
	
	bitmap_init(&group->map);

	mem_node_t *node; 

	/* 根据缓冲中记录的对象大小进行不同的设定 */
	if (cache->object_size < 1024) {
		group->objects = map + 16;

		/* 转换成节点，并标记 */
		node = addr2page(v2p(group));
		CHECK_MEM_NODE(node);
		MEM_NODE_MARK_CHACHE_GROUP(node, cache, group);
	} else {
		unsigned int pages = DIV_ROUND_UP(cache->object_count * cache->object_size, PAGE_SIZE); 
		group->objects = mem_cache_alloc_pages(pages);
		if (group->objects == NULL) {
			printk(KERN_ERR "alloc page for mem objects failed\n");
			return -1;
		}
		int i;
		for (i = 0; i < pages; i++) {
			node = addr2page(v2p(group->objects + i * PAGE_SIZE));
			CHECK_MEM_NODE(node);
			MEM_NODE_MARK_CHACHE_GROUP(node, cache, group);
		}
	}

	group->using_count = 0;
	group->free_count = cache->object_count;
	group->flags =  flags;

	//dump_mem_group(group);
	return 0;
}
/*
 * groupCreate - 创建一个新的group
 * @cache: group所在的cache
 * @flags:创建的标志
 * 
 * 如果成功返回，失败返回-1
 */
static int create_mem_group(mem_cache_t *cache, flags_t flags)
{
	mem_group_t *group;

    //printk("cache %s need a new group %x!\n", cache->name, cache->object_size);

	/* 为内存组分配一个页 */
	group = mem_cache_alloc_pages(1);
	if (group == NULL) {
		printk(KERN_ERR "alloc page for mem group failed!\n");
		return -1;
	}

	if (mem_group_init(cache, group, flags)) {
		printk(KERN_ERR "init mem group failed!\n");
		goto free_group;
	}
	
	// 创建成功
	return 0;

free_group:
	// 释放对象组的页
	mem_cache_free_pages(group);
	return -1;
}


static int make_mem_caches()
{
	// 指向cache大小的指针
	cache_size_t *cachesz = cache_size;

	// mem_cache_table
	mem_cache_t *mem_cache = &mem_cache_table[0];

	//printk("mem_cache_table addr %x size %d\n", mem_cache, sizeof(mem_cache_t));

	// 如果没有遇到大小为0的cache，就会把cache初始化
	while (cachesz->cache_size) {
		/* 初始化缓存信息 */
		if (mem_cache_init(mem_cache, "mem cache", cachesz->cache_size, 0)) {
			printk("create mem cache failed!\n");
			return -1;
		}
		
		// 设定cachePtr
		cachesz->mem_cache = mem_cache;

		// 指向下一个mem cache
		mem_cache++;

		// 指向下一个cache size
		cachesz++;
	}

	return 0;
}

/*
 * __mem_cache_alloc_object - 在group中分配一个对象
 * @cache: 对象所在的cache
 * @group: 对象所在的group
 * 
 * 在当前cache中分配一个对象
 */
static void *__mem_cache_alloc_object(mem_cache_t *cache, mem_group_t *group)
{
	void *object;

	// 做一些标志的设定

	
	// 从位图中获取一个空闲的对象
	int idx = bitmap_scan(&group->map, 1);
	
	// 分配失败
	if (idx == -1) {
		/* 没有可用对象了 */
		printk(KERN_EMERG "bitmap scan failed!\n");
		
		return NULL;
	}
		
	// 设定为已经使用
	bitmap_set(&group->map, idx, 1);

	// 获取object的位置
	object = group->objects + idx * cache->object_size;
	
	// 改变group的使用情况
	group->using_count++;
	group->free_count--;
	
	// 判断group是否已经使用完了
	if (group->free_count == 0) {
		// 从partial的这链表删除
		list_del(&group->list);
		// 添加到full中去
		list_add_tail(&group->list, &cache->full_groups);
	}

	return object;
}

/*
 * mem_cache_alloc_object - 在group中分配一个对象
 * @cache: 对象所在的cache
 * @flags: 分配需要的标志
 * 
 * 在当前cache中分配一个对象
 */
void *mem_cache_alloc_object(mem_cache_t *cache)
{
	void *object;
	mem_group_t *group;

	// 存在空闲的就分配并且返回
	list_t *partialList, *node;

	// 检测分配环境

	unsigned long flags;

retry_alloc_object:
	// 要关闭中断，并保存寄存器环境
    save_intr(flags);

	partialList = &cache->partial_groups;

	// 指向partial中的第一个group
	node = partialList->next;

	//printk("cache size %x\n", cache->object_size);

	// 如果partial是空的
	if (list_empty(partialList)) {
		//printk("partialList empty\n");

		list_t *freeList;
		freeList = &cache->free_groups;
		// 如果free是空的
		if (list_empty(freeList)) {
			//printk("free empty\n");

			// 需要创建一个新的group
			goto new_group;
		}
		// 指向第一个组
		node = freeList->next;

		// 把node从free list中删除
		list_del(node);

		// 把node添加到partial中去
		list_add_tail(node, partialList);

	}

	//printk("kmalloc: found a node.\n");
	/* 现在node是partial中的一个节点 */
	group = list_owner(node, mem_group_t, list);
	//printk("group %x cache size %x\n", group, cache->object_size);
	object = __mem_cache_alloc_object(cache, group);

	// 要恢复中断状态
    restore_intr(flags);
	return object;
new_group:
	// 没有group，添加一个新的group

	// 恢复中断状况
	// 要恢复中断状态
	restore_intr(flags);
	
	//printk("kmalloc: need a new group.\n");
	// 添加新的group
	if (create_mem_group(cache, 0))
		return NULL;	// 如果创建一个group失败就返回

	goto retry_alloc_object;
	return NULL;
}

/*
 * kmalloc - 分配一个对象
 * @size: 对象的大小
 * @flags: 分配需要的flags
 * 
 * 分配一个size大小的内存
 */
void *kmalloc(size_t size)
{
	// 如果越界了就返回空
	if (size > MAX_MEM_CACHE_SIZE) {
		printk(KERN_NOTICE "kmalloc size %d too big!", size);
		return NULL;
	}
	
	cache_size_t *cachesz = &cache_size[0];

	while (cachesz->cache_size) {
		// 如果找到一个大于等于当前大小的cache，就跳出循环
		if (cachesz->cache_size >= size)
			break;
		
		// 指向下一个大小描述
		cachesz++;
	}
	
	//printk("des %x cache %x size %x\n", sizeDes, sizeDes->cachePtr, sizeDes->cachePtr->object_size);
	return mem_cache_alloc_object(cachesz->mem_cache);
}


/*
 * __mem_cache_free_object - 释放一个group对象
 * @cache: 对象所在的cache
 * @object: 对象的指针
 * 
 * 释放group对象，而不是group，group用destory
 */
static void __mem_cache_free_object(mem_cache_t *cache, void *object)
{
	mem_group_t *group;

	// 获取group

	// 获取页
	mem_node_t *node = addr2page(v2p(object));

	CHECK_MEM_NODE(node);

	//printk("OBJECT %x page %x cache %x", object, page, page->groupCache);
	// 遍历cache的3个链表。然后查看group是否有这个对象的地址
	group = MEM_NODE_GET_GROUP(node);

	// 如果查询失败，就返回，代表没有进行释放
	if (group == NULL) 
		panic(KERN_EMERG "group get from page bad!\n");

	//printk("get object group %x\n", group);

	// 把group中对应的object释放，也就是把位图清除

	// 找到位图的索引
	int index = (((unsigned char *)object) - group->objects)/cache->object_size; 
	
	// 检测index是否正确
	if (index < 0 || index > group->map.byte_length*8)
		panic(KERN_EMERG "map index bad range!\n");
	
	// 把位图设置为0，就说明它没有使用了
	bitmap_set(&group->map, index, 0);

	int unsing = group->using_count;
	/*
	dump_mem_group(group);
	DumpMemNode(node);*/

	//printk("kfree: found group and node.\n");

	// 使用中的对象数减少
	group->using_count--;
	// 空闲的对象数增加
	group->free_count++;
	
	// 没有使用中的对象
	if (!group->using_count) {
		// 把它放到free空闲列表
		list_del(&group->list);
		list_add_tail(&group->list, &cache->free_groups);

		//printk("kfree: free to free group.\n");
		//memset(group->map.bits, 0, group->map.byte_length);
		//printk("free to cache %x name %s group %x\n", cache, cache->name, group);
	} else if (unsing == cache->object_count) {
		// 释放之前这个是满的group，现在释放后，就到partial中去
		list_del(&group->list);
		list_add_tail(&group->list, &cache->partial_groups);
		
		//printk("kfree: free to partial group.\n");
	}
}

/*
 * mem_cache_free_object - 释放一个group对象
 * @cache: 对象所在的cache
 * @object: 对象的指针
 * 
 * 释放group对象，而不是group，group用destory
 */
void mem_cache_free_object(mem_cache_t *cache, void *object)
{
	// 检测环境

	// 关闭中断
	unsigned long flags;
    save_intr(flags);

	__mem_cache_free_object(cache, object);

	// 打开中断
    restore_intr(flags);
}


/*
 * kfree - 释放一个对象占用的内存
 * @object: 对象的指针
 */
void kfree(void *objcet)
{
	if (!objcet)
		return;
	mem_cache_t *cache;
	
	// 获取对象所在的页
	mem_node_t *node = addr2page(v2p(objcet));

	CHECK_MEM_NODE(node);
	
	// 转换成group cache
	cache = MEM_NODE_GET_CACHE(node);

	//dump_mem_cache(cache);
	//printk("get object group cache %x\n", cache);

	// 调用核心函数
	mem_cache_free_object(cache, (void *)objcet);
	
}


/*
 * group_destory - 销毁group
 * @cache: group所在的cache
 * @group: 要销毁的group
 * 
 * 销毁一个group,成功返回0，失败返回-1
 */
static int group_destory(mem_cache_t *cache, mem_group_t *group)
{
	// 删除链表关系
	list_del(&group->list);
	
	/* 根据缓冲中记录的对象大小进行不同的设定 */
	if (cache->object_size < 1024) {
		/* 只释放group所在的内存，因为所有数据都在里面 */
		if (mem_cache_free_pages(group))
			return -1;
	} else {
		/* 要释放group和对象所在的页 */
		if (mem_cache_free_pages(group->objects))
			return -1;
		if (mem_cache_free_pages(group))
			return -1;
	}
	return 0;
}

/**
 * __mem_cache_shrink - 收缩内存大小 
 * @cache: 收缩哪个缓冲区的大小
 */
static int __mem_cache_shrink(mem_cache_t *cache)
{
	mem_group_t *group, *next;

	int ret = 0;

	list_for_each_owner_safe(group, next, &cache->free_groups, list) {
		// 销毁成功才计算销毁数量
		//printk("find a free group %x\n", group);
		if(!group_destory(cache, group))
			ret++;
	}
	return ret;
}

/**
 * SlabCacheShrink - 收缩内存大小 
 * @cache: 收缩哪个缓冲区的大小
 */
static int mem_cahce_shrink(mem_cache_t *cache)
{
	int ret;
	// cache出错的话就返回
	if (!cache) 
		return 0; 

	// 用自旋锁来保护结构
	unsigned long flags;
    save_intr(flags);
    // 收缩内存
	ret = __mem_cache_shrink(cache);

	// 打开锁
    restore_intr(flags);

	// 返回收缩了的内存大小
	return ret * cache->object_count * cache->object_size;
}

/**
 * SlabCacheAllShrink - 对所有的cache都进行收缩
 */
int kmshrink()
{
	// 释放了的大小
	size_t size = 0;

	// 指向cache的指针
	cache_size_t *cachesz = &cache_size[0];

	// 对每一个cache都进行收缩
	while (cachesz->cache_size) {
		// 收缩大小
		size += mem_cahce_shrink(cachesz->mem_cache);
		// 指向下一个cache大小描述
		cachesz++;
	}

	return size;
}

int init_mem_caches()
{
    /* make basic mem caches */
	make_mem_caches();
#if 0       /* test */
	char *a = kmalloc(32);
	char *b = kmalloc(512);
	char *c = kmalloc(1024);
	char *d = kmalloc(128*1024);
	
	//memset(d, 0, 512*1024);

	a = kmalloc(32);
	b = kmalloc(512);
	c = kmalloc(1024);
	d = kmalloc(128*1024);

	printk("a=%x, b=%x,c=%x,d=%x,\n", a, b,c,d);

	kfree(a);
	kfree(b);
	kfree(c);
	kfree(d);
	a = kmalloc(32);
	b = kmalloc(512);
	c = kmalloc(1024);
	d = kmalloc(128*1024);
    
	printk("a=%x, b=%x,c=%x,d=%x,\n", a, b,c,d);

	kfree(a);
	kfree(b);
	kfree(c);
	kfree(d);
	
	a = kmalloc(64);
	b = kmalloc(256);
	c = kmalloc(4096);
	d = kmalloc(64*1024);

	printk("a=%x, b=%x,c=%x,d=%x,\n", a, b,c,d);

	int i = 0;

	char *table[10];
	for (i = 0; i < 10; i++) {
		table[i] = kmalloc(128*1024);
		printk("x=%x\n", table[i]);
	}

	for (i = 0; i < 10; i++) {
		kfree(table[i]);
	}

	size_t size = kmshrink();
	printk("shrink size %x bytes %d MB\n", size, size/MB);

	for (i = 0; i < 10; i++) {
		table[i] = kmalloc(1*MB);
		printk("x=%x\n", table[i]);
	}

	for (i = 0; i < 10; i++) {
		kfree(table[i]);
	}
    size = kmshrink();
	printk("shrink size %x bytes %d MB\n", size, size/MB);
	

    mem_cache_t newcache;
    mem_cache_init(&newcache, "new cache", 10 * MB, 0);

    void *na = mem_cache_alloc_object(&newcache);
    if (na == NULL) 
        printk("alloc failed\n");
    
    memset(na, 0, 10 * MB);
    printk("alloc at %p\n", na);
    
    void *nb = mem_cache_alloc_object(&newcache);
    if (nb == NULL) 
        printk("alloc failed\n");
    
    memset(nb, 0, 10 * MB);
    
    printk("alloc at %p\n", nb);
    
    mem_cache_free_object(&newcache, na);
    mem_cache_free_object(&newcache, nb);

    size = kmshrink();
	printk("shrink size %x bytes %d MB\n", size, size/MB);

    unsigned long free_size = get_free_page_nr();
    unsigned long total_size = get_total_page_nr();
    printk("total:%d free:%d used:%d\n", total_size, free_size, total_size - free_size);
	
#endif


	return 0;
}
