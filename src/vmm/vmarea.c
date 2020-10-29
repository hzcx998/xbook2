#include <arch/interrupt.h>
#include <arch/ioremap.h>
#include <xbook/vmarea.h>
#include <xbook/debug.h>
#include <xbook/bitmap.h>
#include <xbook/memcache.h>
#include <string.h>

/* vir addr management bitmap */
static bitmap_t vaddr_bitmap;

/* vaddr start base */
static unsigned long vaddr_base; 

/* 正在使用中的vmarea */
static list_t using_vmarea_list;

/* 处于空闲状态的vmarea，成员根据大小进行排序，越小的就靠在前面 */
static list_t free_vmarea_list;

/**
 * alloc_vaddr - 分配一块空闲的虚拟地址
 * @size: 请求的大小
 * 
 * @return: 失败返回0，成功返回内核虚拟地址
 */
unsigned long alloc_vaddr(size_t size)
{
	size = PAGE_ALIGN(size);
	if (!size)
		return 0;
	
	long pages = size / PAGE_SIZE;

	/* 扫描获取请求的页数 */
	long idx = bitmap_scan(&vaddr_bitmap, pages);
	if (idx == -1)
		return 0;

	long i;
	/* 把已经扫描到的位置1，表明已经分配了 */
	for (i = 0; i < pages; i++) {
		bitmap_set(&vaddr_bitmap, idx + i, 1);
	}

	/* 返还转换好的虚拟地址 */
	return vaddr_base + idx * PAGE_SIZE; 
}

/**
 * free_vaddr - 释放一块空闲的虚拟地址
 * @vaddr: 虚拟地址
 * @size: 占用的大小
 */
unsigned long free_vaddr(unsigned long vaddr, size_t size)
{
    if (!size)
        return -1;
    size = PAGE_ALIGN(size);
	long pages = size / PAGE_SIZE;

	/* 扫描获取请求的页数 */
	long idx = (vaddr - vaddr_base) / PAGE_SIZE;
	if (idx == -1)
		return -1;

	long i;
	/* 把地址对应的位图到的位置0，表明已经释放了 */
	for (i = 0; i < pages; i++) {
		bitmap_set(&vaddr_bitmap, idx + i, 0);
	}

	return 0; 
}

static void *__vmalloc(size_t size)
{
	/* 创建一个新的区域 */
	unsigned long start = alloc_vaddr(size);
	if (!start) 
		return NULL;
	
	vmarea_t *area;

	/* 创建一个虚拟区域 */
	area = kmalloc(sizeof(vmarea_t));
	if (area == NULL) {
		free_vaddr(start, size);
		return NULL;
	}

	area->addr = start;
	area->size = size;

	unsigned long flags;
    interrupt_save_state(flags);
	/* 添加到虚拟区域的链表上 */
	list_add_tail(&area->list, &using_vmarea_list);

	if (page_map_addr(start, size, PROT_KERN | PROT_WRITE)) {
		free_vaddr(start, size);
		kfree(area);
		interrupt_restore_state(flags);
		return NULL;
	}
	interrupt_restore_state(flags);
    //printk("vmalloc: create a area %x/%x\n", area->addr, area->size);
	return (void *)area->addr;
}

/**
 * vmalloc - 分配空间
 * @size: 空间的大小
 */
void *vmalloc(size_t size)
{
	size = PAGE_ALIGN(size);

	if (!size)
		return NULL;

	/* 中间用一个页隔离 */
	size += PAGE_SIZE;

	//printk("size %d\n", size);

	vmarea_t *target = NULL, *area;

	/* 先从空闲链表中查找 */
	list_for_each_owner(area, &free_vmarea_list, list) {
		/* 如果找到了大小合适的区域 */
		if (size >= area->size) {
			target = area;
			break;
		}
	}

	/* 找到一个合适大小的area，就使用它 */
	if (target != NULL) {
		//printk("vmalloc: find a free area %x/%x\n", target->addr, target->size);
		unsigned long flags;
        interrupt_save_state(flags);

		/* 先脱离原来的空闲链表，并添加到使用链表中去 */
		list_del(&target->list);

		list_add_tail(&target->list, &using_vmarea_list);
		interrupt_restore_state(flags);
		return (void *)target->addr;
	}

	return (void *)__vmalloc(size);
}

static int __vfree(vmarea_t *target)
{
	vmarea_t *area;

	//printk("vfree: find a using area %x/%x\n", target->addr, target->size);
	
	/* 先脱离原来的使用链表，并添加到空闲链表中去 */
	list_del(&target->list);

	/* 成功插入标志 */
	char insert = 0;	
	/* 这里要根据area大小进行排序，把最小的排在最前面 */
	if (list_empty(&free_vmarea_list)) {
		//printk("vfree: free area is empty %x/%x\n", target->addr, target->size);
		/* 链表是空，直接添加到最前面 */
		list_add(&target->list, &free_vmarea_list);
	} else {
		//printk("vfree: free area is not empty %x/%x\n", target->addr, target->size);
		/* 获取第一个宿主 */
		area = list_first_owner(&free_vmarea_list, vmarea_t, list);

		do {
			/* 根据大小来判断插入位置，小的在前面，大的在后面 */
			if (target->size > area->size) {
				/*printk("target %x/%x area %x/%x\n",
					target->addr, target->size, area->addr, area->size);*/

				/* 不满足条件，需要继续往后查找 */
				if (area->list.next == &free_vmarea_list) {
					/* 如果到达了最后面，也就是说下一个就是链表头了
						直接把target添加到队列的最后面
						*/
					list_add_tail(&target->list, &free_vmarea_list);
					insert = 1;
					//printk("vfree: insert tail %x/%x\n");
					break;
				}

				/* 获取下一个宿主 */
				area = list_owner(area->list.next, vmarea_t, list);
				
			} else {
				/* 插入到中间的情况 */

				/* 把新节点添加到旧节点前面 */
				list_add_before(&target->list, &area->list);
				insert = 1;
				//printk("vfree: insert before area %x/%x\n", area->addr, area->size);

				break;
			}
		} while (&area->list != &free_vmarea_list);
	}

	return insert;
}


/**
 * vfree - 释放空间
 * @ptr: 空间所在的地址
 */
int vfree(void *ptr)
{
	if (ptr == NULL)
		return -1;

	unsigned long addr = (unsigned long)ptr;

	if (addr < vaddr_base || addr >= VMAREA_END)
		return -1;
	
	vmarea_t *target = NULL, *area;
	unsigned long flags;
    interrupt_save_state(flags);
	list_for_each_owner(area, &using_vmarea_list, list) {
		/* 如果找到了对应的区域 */
		if (area->addr == addr) {
			target = area;
			break;
		}
	}

	/* 找到一个合适要释放的area，就释放它 */
	if (target != NULL) {
		if (__vfree(target)) {
			interrupt_restore_state(flags);
			return 0;
		}
	}
	
	/* 没找到，释放失败 */
    interrupt_restore_state(flags);
	return -1;
}


/**
 * ioremap - io内存映射
 * @paddr: 物理地址
 * @size: 映射的大小
 * 
 * @return: 成功返回映射后的地址，失败返回NULL
 */
void *ioremap(unsigned long paddr, size_t size)
{
    /* 对参数进行检测,地址不能是0，大小也不能是0 */
    if (!paddr || !size) {
        return NULL;
    }

    /* 分配虚拟地址 */
    unsigned long vaddr = alloc_vaddr(size);
    if (vaddr == -1) {
        printk("alloc virtual addr for IO remap failed!\n");
        return NULL;
    }
    //printk("alloc a virtual addr at %x\n", vaddr);

    /* 创建一个虚拟区域 */
	vmarea_t *area;

	/* 创建一个虚拟区域 */
	area = kmalloc(sizeof(vmarea_t));
	if (area == NULL) {
		free_vaddr(vaddr, size);
		return NULL;
	}
    /* 设置虚拟区域参数 */
	area->addr = vaddr;
	area->size = size;
    unsigned long flags;
    interrupt_save_state(flags);

	/* 添加到虚拟区域的链表上 */
	list_add_tail(&area->list, &using_vmarea_list);
    
    /* 进行io内存映射，如果失败就释放资源 */
    if (phy_addr_remap(paddr, vaddr, size)) {
        /* 释放分配的资源 */
        list_del(&area->list);
        kfree(area);
        free_vaddr(vaddr, size);
        /* 指向0，表示空 */
        vaddr = 0;
    }
    interrupt_restore_state(flags);

    return (void *)vaddr;    
}

/**
 * iounmap - 取消io内存映射
 * @vaddr: 虚拟地址地址
 * 
 * @return: 成功返回映射后的地址，失败返回NULL
 */
int iounmap(void *vaddr)
{
    if (vaddr == NULL) {
        return -1;
    }
    unsigned long addr = (unsigned long )vaddr;
    
	if (addr < vaddr_base || addr >= VMAREA_END)
		return -1;
	
	vmarea_t *target = NULL, *area;
	unsigned long flags;
    interrupt_save_state(flags);

	list_for_each_owner(area, &using_vmarea_list, list) {
		/* 如果找到了对应的区域 */
		if (area->addr == addr) {
			target = area;
			break;
		}
	}

	/* 找到一个合适要释放的area，就释放它 */
	if (target != NULL) {
        if (phy_addr_unmap(target->addr, target->size)) {
		    /* 取消IO映射并释放area */
            
            list_del(&target->list);

            free_vaddr(addr, target->size);

            kfree(target);

            interrupt_restore_state(flags);
            return 0;
		}
	}
	
	/* 没找到，释放失败 */
	interrupt_restore_state(flags);
    return -1;
}

/**
 * init_vmarea - 初始化虚拟区域
 */
void init_vmarea()
{
	/* 每一位代表1个页的分配状态 */
	vaddr_bitmap.byte_length = DYNAMIC_MAP_MEM_SIZE / (PAGE_SIZE * 8);
	
	/* 为位图分配空间 */
	vaddr_bitmap.bits = kmalloc(vaddr_bitmap.byte_length);

	bitmap_init(&vaddr_bitmap);

	vaddr_base = VMAREA_BASE;

	/* 初始化使用中的区域链表 */
	INIT_LIST_HEAD(&using_vmarea_list);

	/* 初始化空闲的区域链表 */
	INIT_LIST_HEAD(&free_vmarea_list);
#if 0
    char *a = vmalloc(PAGE_SIZE);
	if (a == NULL)
		printk("vmalloc failed!\n");

	memset(a, 0, PAGE_SIZE);

	
	char *b = vmalloc(PAGE_SIZE* 10);
	if (b == NULL)
		printk("vmalloc failed!\n");

	memset(b, 0, PAGE_SIZE *10);

	
	char *c = vmalloc(PAGE_SIZE *100);
	if (c == NULL)
		printk("vmalloc failed!\n");

	memset(c, 0, PAGE_SIZE *100);

	char *d = vmalloc(PAGE_SIZE *1000);
	if (d == NULL)
		printk("vmalloc failed!\n");

	memset(d, 0, PAGE_SIZE *1000);

	printk("%x %x %x %x\n", a, b, c, d);

	vfree(a);
	vfree(b);
	vfree(c);
	vfree(d);
	
	a = vmalloc(PAGE_SIZE);
	if (a == NULL)
		printk("vmalloc failed!\n");

	memset(a, 0, PAGE_SIZE);
	
	b = vmalloc(PAGE_SIZE* 10);
	if (b == NULL)
		printk("vmalloc failed!\n");

	memset(b, 0, PAGE_SIZE *10);

	c = vmalloc(PAGE_SIZE *100);
	if (c == NULL)
		printk("vmalloc failed!\n");

	memset(c, 0, PAGE_SIZE *100);

	d = vmalloc(PAGE_SIZE *1000);
	if (d == NULL)
		printk("vmalloc failed!\n");

	memset(d, 0, PAGE_SIZE *1000);

	printk("%x %x %x %x\n", a, b, c, d);

	vfree(c);
	vfree(b);
	vfree(d);
	vfree(a);
    void *mapa;
    if ((mapa = ioremap(0xeff00000, 1 * MB)) == NULL) {
        printk("ioremap failed!");
    }

    memset(mapa, 0, 1 * MB);
    printk("ioremap ok!\n");
    iounmap(mapa);

    printk("iounmap ok!\n");
    memset(mapa, 0, 1 * MB);
#endif
    
}
