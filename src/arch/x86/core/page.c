#include "page.h"
#include "pmem.h"
#include "instruction.h"
#include <xbook/debug.h>
#include <xbook/math.h>
#include <xbook/memops.h>
#include <xbook/assert.h>
#include <xbook/task.h>
#include <xbook/vmspace.h>

extern mem_node_t *mem_node_table;
extern unsigned int mem_node_count;
extern unsigned int mem_node_base;

unsigned long __alloc_pages(unsigned long count)
{
    mem_node_t *node = get_free_mem_node();
    if (node == NULL)
        return 0;
    
    int index = node - mem_node_table;

    /* 如果分配的页数超过范围 */
    if (index + count > mem_node_count)
        return 0; 

    /* 第一次分配的时候设置引用为1 */
    node->reference = 1;
    node->count = (unsigned int)count;
    node->flags = 0;
    node->cache = NULL;
    node->group = NULL;

    return (long)__mem_node2page(node);
}

int __free_pages(unsigned long page)
{
    mem_node_t *node = __page2mem_node((unsigned long)page);

    if (node == NULL)
        return -1;
    
    __atomic_dec(&node->reference);

	if (node->reference == 0) {
        node->count = 0;
		node->flags = 0;
        return 0;
	}
    return -1;
}


/*
 * __page_link - 物理地址和虚拟地址链接起来
 * @va: 虚拟地址
 * @pa: 物理地址
 * @prot: 页的保护
 * 
 * 把虚拟地址和物理地址连接起来，这样就能访问物理地址了
 */
void __page_link(unsigned long va, unsigned long pa, unsigned long prot)
{
    unsigned long vaddr = (unsigned long )va;
    unsigned long paddr = (unsigned long )pa;
    /* get page dir and page table */
	pde_t *pde = get_pde_ptr(vaddr);
	pte_t *pte = get_pte_ptr(vaddr);

    /* if page table exist. */
	if (*pde & PG_P_1) {
        /* phy page must not exist! */
        ASSERT(!(*pte & PG_P_1)); 

        /* make sure phy page not exist! */
        if (!(*pte & PG_P_1)) {
            *pte = (paddr | prot | PG_P_1);
        } else {
            /* phy page exist! */
            panic("pte %x has exist!\n", *pte);
            *pte = (paddr | prot | PG_P_1);
        }
	} else { /* no page table, need create a new one. */
        unsigned long page_table = __alloc_pages(1);
        if (!page_table) {
            /* no page left */
            panic("kernel no page left!\n");
            /* we can goto free some pages and try it again,
             but now we just stop! */
        }
        //printk(KERN_DEBUG "page_link -> new page table %x\n", page_table);

        /* add page table to page dir */
        *pde = (page_table | prot | PG_P_1);

        /* clear page to avoid dirty data */
        memset((void *)((unsigned long)pte & PAGE_ADDR_MASK), 0, PAGE_SIZE);

        /* make sure phy page not exist! */
        ASSERT(!(*pte & PG_P_1));

        *pte = (paddr | prot | PG_P_1);
    }
}


/*
 * __page_link_unsafe - 物理地址和虚拟地址链接起来
 * @va: 虚拟地址
 * @pa: 物理地址
 * @prot: 页的保护
 * 
 * 把虚拟地址和物理地址连接起来，这样就能访问物理地址了
 */
void __page_link_unsafe(unsigned long va, unsigned long pa, unsigned long prot)
{
    unsigned long vaddr = (unsigned long )va;
    unsigned long paddr = (unsigned long )pa;
    /* get page dir and page table */
	pde_t *pde = get_pde_ptr(vaddr);
	pte_t *pte = get_pte_ptr(vaddr);

    /* if page table exist. */
	if (*pde & PG_P_1) {
        /* phy page must not exist! */
        //ASSERT(!(*pte & PG_P_1));

        if ((*pte & PG_P_1)) {
            free_page(*pte & PAGE_ADDR_MASK);
            *pte = 0;
        }

        /* make sure phy page not exist! */
        if (!(*pte & PG_P_1)) {
            *pte = (paddr | prot | PG_P_1);
        } else {
            /* phy page exist! */
            panic("pte %x has exist!\n", *pte);
            *pte = (paddr | prot | PG_P_1);
        }
	} else { /* no page table, need create a new one. */
        unsigned long page_table = __alloc_pages(1);
        if (!page_table) {
            /* no page left */
            panic("kernel no page left!\n");
            /* we can goto free some pages and try it again,
             but now we just stop! */
        }
        //printk(KERN_DEBUG "page_link -> new page table %x\n", page_table);

        /* add page table to page dir */
        *pde = (page_table | prot | PG_P_1);

        /* clear page to avoid dirty data */
        memset((void *)((unsigned long)pte & PAGE_ADDR_MASK), 0, PAGE_SIZE);

        /* make sure phy page not exist! */
        //ASSERT(!(*pte & PG_P_1));

        if ((*pte & PG_P_1)) {
            free_page(*pte & PAGE_ADDR_MASK);
            *pte = 0;
        }

        *pte = (paddr | prot | PG_P_1);
    }
}

/*
 * __page_unlink - 取消虚拟地址对应的物理链接
 * @vaddr: 虚拟地址
 * 
 * 取消虚拟地址和物理地址链接。
 * 在这里我们不删除页表项映射，只删除物理页，这样在以后映射这个地址的时候，
 * 可以加速运行。但是弊端在于，内存可能会牺牲一些。
 * 也消耗不了多少，4G内存才4MB。
 */
void __page_unlink(unsigned long vaddr)
{
	pte_t *pte = get_pte_ptr(vaddr);

	// 如果页表项存在物理页
	if (*pte & PG_P_1) {
		//printk(KERN_DEBUG "unlink vaddr:%x pte:%x\n", vaddr, *pte);
		
        // 清除页表项的存在位，相当于删除物理页
		*pte &= ~PG_P_1;

		/* flush vaddr tbl cache */
        flush_tbl(vaddr);        
    }
}

/**
 * __map_pages - 映射页面
 * 
 * 非连续内存使用，可以提高分配效率
 */
int __map_pages(unsigned long start, unsigned long len, unsigned long prot)
{
    unsigned long first = start & PAGE_MASK;    /* 页地址对齐 */
    // 长度和页对齐
    len = PAGE_ALIGN(len);

    /* set page attr */
    unsigned long attr = 0;
    
    if (prot & PROT_USER)
        attr |= PG_US_U;
    else
        attr |= PG_US_S;
    
    if (prot & PROT_WRITE)
        attr |= PG_RW_W;
    else
        attr |= PG_RW_R;

	/* 判断长度是否超过剩余内存大小 */
    //printk("len %d pages %d\n", len, len / PAGE_SIZE);

	/* 分配物理页 */
	unsigned long pages = __alloc_pages(len / PAGE_SIZE);

	if (!pages) {
        printk(KERN_ERR "map_pages -> map without free pages!\n");
        return -1;
    }
	unsigned long end = first + len;
	//printk(KERN_DEBUG "map_pages -> start%x->%x len %x\n", first, pages, len);
    while (first < end)
	{
        // 对单个页进行链接
		__page_link(first, pages, attr);
		first += PAGE_SIZE;
        pages += PAGE_SIZE;
	}
	return 0;
}


/**
 * __map_pages_fixed - 映射固定页面
 * @start: 起始虚拟地址
 * @addr: 物理地址
 * @len: 映射长度
 * @prot: 映射的属性
 * 
 * 固定的物理地址和虚拟地址映射
 * 
 * @return: 成功返回0，失败返回-1
 */
int __map_pages_fixed(unsigned long start, unsigned long addr, unsigned long len, unsigned long prot)
{
    unsigned long first = start & PAGE_MASK;    /* 页地址对齐 */
    // 长度和页对齐
    len = PAGE_ALIGN(len);

    /* set page attr */
    unsigned long attr = 0;
    
    if (prot & PROT_USER)
        attr |= PG_US_U;
    else
        attr |= PG_US_S;
    
    if (prot & PROT_WRITE)
        attr |= PG_RW_W;
    else
        attr |= PG_RW_R;

	/* 判断长度是否超过剩余内存大小 */
    //printk("len %d pages %d\n", len, len / PAGE_SIZE);

	/* 获取物理页地址 */
	unsigned long pages = addr;
    
    unsigned long end = first + len;
	//printk(KERN_DEBUG "map_pages -> start%x->%x len %x\n", first, pages, len);
    while (first < end)
	{
        if (prot & PROT_REMAP) {
            __page_link_unsafe(first, pages, attr);
        } else {
            // 对单个页进行链接
		    __page_link(first, pages, attr);
        }
        first += PAGE_SIZE;
        pages += PAGE_SIZE;
	}
	return 0;
}

/**
 * __addr_v2p - 虚拟地址转换为物理地址
 * 
 * 用于非连续地址转换。 
 */
unsigned long __addr_v2p(unsigned long vaddr)
{
	pte_t* pte = get_pte_ptr(vaddr);
	/* 
	(*pte)的值是页表所在的物理页框地址,
	去掉其低12位的页表项属性+虚拟地址vaddr的低12位
	*/
	return ((*pte & 0xfffff000) + (vaddr & 0x00000fff));
}

/**
 * __unmap_pages - 取消页映射
 * @vaddr: 虚拟地址
 * @len: 内存长度
 * 
 * 和map_pages配合使用，同使用于非连续内存
 */
int __unmap_pages(unsigned long vaddr, unsigned long len)
{
	if (!len)
		return -1;
	
	len = PAGE_ALIGN(len);
	
	/* 判断长度是否超过剩余内存大小 */
	unsigned long paddr, start = (unsigned long)vaddr & PAGE_MASK;    /* 页地址对齐 */

    unsigned long end = start + len;
	
    /* 不能通过__v2p来获取，需要从页表映射中解析获取 */
	paddr = __addr_v2p(start);
    
	//printk("unmap pages:%x->%x len %x\n", vaddr, paddr, len);
	while (start < end)
	{
		__page_unlink(start);
		start += PAGE_SIZE;
	}
	
	// 释放物理页
	__free_pages(paddr);
    
	return 0;
}

/**
 * __map_pages_safe - 安全地映射页
 * 
 * 如果页已经映射，就不覆盖，直接跳过
 * 每次映射单个页，效率较低，不过比较安全
 * 用于进程地址空间
 */
int __map_pages_safe(unsigned long start, unsigned long len, unsigned long prot)
{
    unsigned long vaddr = (unsigned long )start & PAGE_MASK;    /* 页地址对齐 */
    len = PAGE_ALIGN(len);
    unsigned long pages = DIV_ROUND_UP(len, PAGE_SIZE);
    unsigned long page_idx = 0;
    unsigned long page_addr;
    
    /* set page attr */
    unsigned long attr = 0;
    
    if (prot & PROT_USER)
        attr |= PG_US_U;
    else
        attr |= PG_US_S;
    
    if (prot & PROT_WRITE)
        attr |= PG_RW_W;
    else
        attr |= PG_RW_W;

    while (page_idx < pages) {
        /* get page dir and page table */
        pde_t *pde = get_pde_ptr(vaddr);
        pte_t *pte = get_pte_ptr(vaddr);

        if (!(*pde & PG_P_1) || !(*pte & PG_P_1)) {
            page_addr = __alloc_pages(1);
            if (!page_addr) {
                printk("error: user_map_vaddr -> map pages failed!\n");
                return -1;
            }
            
            __page_link(vaddr, page_addr, attr);
            
            //printk("info: map_pages_safe -> start%x->%x\n", vaddr, page_addr);
        }
        vaddr += PAGE_SIZE;
        page_idx++;
    }
    return 0;
}

static int is_page_table_empty(pte_t *page_table)
{
    int i;
    for (i = 0; i < 0; i++) {
        /* if some one exist, not empty */
        if (page_table[i] & PG_P_1)
            return 0;
    }
    return 1;   /* empty */
}

/**
 * __unmap_pages_safe - 安全地取消映射页
 * @start: 起始地址
 * @len: 范围长度
 * @fixed: 是否为固定页，0不是，1是。
 * 
 * 如果页已经映射，就不覆盖，直接跳过，每次释放单个页，可以释放所有资源。
 * 用于进程地址释放。
 * fixed用于判断是否是固定页，固定页的意思是，映射的时候，虚拟地址和一个显示的指出物理地址
 * 进行映射。非固定页就是，映射的时候只指定了虚拟地址，而物理地址是随机分配的。
 * 
 * @return: 成功返回0，失败返回-1
 */
int __unmap_pages_safe(unsigned long start, unsigned long len, char fixed)
{
    unsigned long vaddr = (unsigned long )start & PAGE_MASK;    /* 页地址对齐 */
    len = PAGE_ALIGN(len);
    unsigned long pages = DIV_ROUND_UP(len, PAGE_SIZE);

    unsigned long pte_idx;       /* pte -> physic page */
    
    pde_t *pde;
    pte_t *pte;
    /*if (fixed)
        printk(KERN_NOTICE "__unmap_pages_safe: fixed %d %x!\n", current_task->pid, start);
    */
    while (pages > 0) {
        pde = get_pde_ptr(vaddr); /* get pde from vaddr  */
        if ((*pde & PG_P_1)) {  /* page table exist */
            /* when page tabel entry nr < 1024, continue free */
            while ((pte_idx = PTE_IDX(vaddr)) < 1024) {
                //printk("info: unmap_pages_safe -> pte idx %d\n", pte_idx);
                pte = get_pte_ptr(vaddr); /* get pte from vaddr  */
                if (*pte & PG_P_1) {
                    //printk("info: unmap_pages_safe -> start%x->%x\n", vaddr, *pte & PAGE_ADDR_MASK);
                    /* free physic page if not fixed */
                    if (!fixed)
                        __free_pages(*pte & PAGE_ADDR_MASK);

                    /* del page entry */
                    *pte &= ~PG_P_1;
                }
                vaddr += PAGE_SIZE;
                pages--;
                if (!pages) { /* no page left  */
                    /* check page table */
                    if (is_page_table_empty((pte_t *)((unsigned long)pte & PAGE_ADDR_MASK))) {
                        //printk("info: unmap_pages_safe -> del page table %x\n", *pde & PAGE_ADDR_MASK);

                        /* free page table */
                        __free_pages(*pde & PAGE_ADDR_MASK);            
                    }

                    goto end;
                }
                /* if at last one, break out */
                if (pte_idx == 1023)
                    break;
            }

            //printk("info: unmap_pages_safe -> del page table %x\n", *pde & PAGE_ADDR_MASK);

            /* free page dir table */
            __free_pages(*pde & PAGE_ADDR_MASK);

            /* del page entry */
            *pde &= ~PG_P_1;
        } else {
            vaddr += PAGE_SIZE;
            pages--;
            if (!pages) { /* no page left  */
                goto end;
            }
        }
    }
end:
    return 0;
}

/* 复制一份内核页，并清空用户态数据 */
unsigned long *__copy_kernel_page_dir()
{
    unsigned long page = __alloc_pages(1);
    unsigned int *vaddr = (unsigned int *)__va(page);
    
    memset(vaddr, 0, PAGE_SIZE);
    memcpy((void *)((unsigned char *)vaddr + PAGE_SIZE / 2), 
        (void *)((unsigned char *)PAGE_DIR_VIR_ADDR + PAGE_SIZE / 2), 
        PAGE_SIZE / 2);

    vaddr[1023] = page | PG_P_1 | PG_US_S | PG_RW_W;
    return (unsigned long *)vaddr;
}

/*
 * mem_self_mapping - 内存自映射
 * @start: 开始物理地址
 * @end: 结束物理地址
 * 
 * 把物理和内核虚拟地址进行映射
 */
int mem_self_mapping(unsigned int start, unsigned int end)
{
	/* ----映射静态内存---- */
	//把页目录表设置
	unsigned int *pdt = (unsigned int *)PAGE_DIR_VIR_ADDR;

	// 求出目录项数量
	unsigned int pde_nr = (end - start) / (1024 * PAGE_SIZE);
    
	// 先获取的页数
	unsigned int pte_nr = (end-start)/PAGE_SIZE;

	// 获取页数中余下的数量，就是页表项剩余数
	pte_nr = pte_nr % 1024;
	
	//跳过页表区域中的第一个页表
	unsigned int *pte_addr = (unsigned int *) (PAGE_TABLE_PHY_ADDR + 
			PAGE_SIZE * PAGE_TABLE_PHY_NR);

	int i, j;
	// 根据页目录项来填写
	for (i = 0; i < pde_nr; i++) {
		//填写页目录表项
		//把页表地址和属性放进去
		pdt[512 + PAGE_TABLE_PHY_NR + i] = (unsigned int)pte_addr | PG_P_1 | PG_RW_W | PG_US_S;
		
		for (j = 0; j < PAGE_ENTRY_NR; j++) {
			//填写页页表项

			//把物理地址和属性放进去
			pte_addr[j] = start | PG_P_1 | PG_RW_W | PG_US_S;
			//指向下一个物理页
			start += PAGE_SIZE;
		}
		//指向下一个页表
		pte_addr += PAGE_SIZE;
	}
	// 根据剩余的页表项填写

	//有剩余的我们才填写
	if (pte_nr > 0) {
		// i 和 pte_addr 的值在上面的循环中已经设置
		pdt[512 + PAGE_TABLE_PHY_NR + i] = (unsigned int)pte_addr | PG_P_1 | PG_RW_W | PG_US_S;
		
		// 填写剩余的页表项数量
		for (j = 0; j < pte_nr; j++) {
			//填写页页表项
			//把物理地址和属性放进去
			pte_addr[j] = start | PG_P_1 | PG_RW_W | PG_US_S;
			
			//指向下一个物理页
			start += PAGE_SIZE;
		}
	}

	/* 在开启分页模式之后，我们的内核就运行在高端内存，
	那么，现在我们不能通过低端内存访问内核，所以，我们在loader
	中初始化的0~8MB低端内存的映射要取消掉才行。
	我们把用户内存空间的页目录项都清空 */ 
	
	for (i = 0; i < 512; i++) {
		pdt[i] = 0;
	}

	/* ----映射完毕---- */
	return 0;
}

static inline int handle_no_page(unsigned long addr, unsigned long prot)
{
    /* 映射一个物理页 */
	return __map_pages(addr, PAGE_SIZE, prot);
}

/**
 * make_page_writable - 让pte有写属性
 * @addr: 要设置的虚拟地址
 */
static int make_page_writable(unsigned long addr)
{
	if (addr > USER_VMM_SIZE)
		return -1;

	pde_t *pde = get_pde_ptr(addr);
	pte_t *pte = get_pte_ptr(addr);
	
	/* 虚拟地址的pde和pte都要存在才能去设置属性 */
	if (!(*pde & PG_P_1))
		return -1;

	if (!(*pte & PG_P_1))
		return -1;
	
	/* 标记写属性 */
	*pte |= PG_RW_W;
	return 0;
}

static inline void do_vmarea_fault(unsigned long addr)
{
    /* 如果是在vmarea区域中，就进行页复制，不是的话，就发出段信号。 */
    panic("do_vmarea_fault: at %x not support now!\n", addr);
}

static inline void do_expand_stack(vmspace_t *space, unsigned long addr)
{
	/* 地址和页向下对齐 */
    addr &= PAGE_MASK;
	/* 修改为新的空间开始 */
	space->start = addr;
}


static inline int do_protection_fault(vmspace_t * space, unsigned long addr, int write)
{
	/* 没有写标志，说明该段内存不支持内存写入，就直接返回吧 */
	if (write) {
		printk(KERN_DEBUG "have write protection.\n");
		/* 只有设置写属性正确才能返回 */
		int ret = make_page_writable(addr);
		if (ret) {
            printk(KERN_ALTER "do_protection_fault: touch TRIGHW trigger because page not writable!");    
            trigger_force(TRIGHW, current_task->pid);    
            return -1;
        }
			
		/* 虽然写入的写标志，但是还是会出现缺页故障，在此则处理一下缺页 */
		if (handle_no_page(addr, space->page_prot)) {
            printk(KERN_ALTER "do_protection_fault: touch TRIGHW trigger because hand no page failed!");
            trigger_force(TRIGHW, current_task->pid);
			return -1; 
        }

		return 0;
	} else {
		printk(KERN_DEBUG "no write protection\n");
	}
    //ForceSignal(SIGSEGV, SysGetPid());
	printk(KERN_ALTER "do_protection_fault: touch TRIGHW trigger because page protection!");
    trigger_force(TRIGHW, current_task->pid);
    //panic(KERN_ERR "page protection fault!\n");
    return -1;
}

/**
 * do_page_fault - 处理页故障
 * 
 * 错误码的内容 
 * bit 0: 0 no page found, 1 protection fault
 * bit 1: 0 read, 1 write
 * bit 2: 0 kernel, 1 user
 * 
 * 如果是来自内核的页故障，就会打印信息并停机。
 * 如果是来自用户的页故障，就会根据地址来做处理。
 */
int do_page_fault(trap_frame_t *frame)
{
    task_t *cur = current_task;
    unsigned long addr = 0x00;

    addr = read_cr2(); /* cr2 saved the fault addr */
    //printk(KERN_DEBUG "page fault addr:%x\n", addr);
    
    /* in kernel page fault */
    if (!(frame->error_code & PG_ERR_USER) && addr >= PAGE_OFFSET) {
        printk("task name=%s pid=%d\n", cur->name, cur->pid);
        printk(KERN_EMERG "a memory problem had occured in kernel, please check your code! :(\n");
        printk(KERN_EMERG "page fault at %x.\n", addr);
        dump_trap_frame(frame);
        panic("halt...");
    }
    /* 如果故障地址位于内核中， */
    if (addr >= USER_VMM_SIZE) {
        /* 故障源是用户，说明用户需要访问非连续内存区域，于是复制一份给用户即可 */
        printk(KERN_DEBUG "user pid=%d name=%s access unmaped vmarea area .\n", cur->pid, cur->name);
        dump_trap_frame(frame);
        print_task();
        do_vmarea_fault(addr);
        return -1;
    }
    /* 故障地址在用户空间 */
    vmspace_t *space = vmspace_find(cur->vmm, addr);
    /* 没找到空间，说明是一个未知区域 */
    if (space == NULL) {    
        /* 故障源是用户 */
    
        printk(KERN_ALTER "do_page_fault: user access user unknown space .\n");
        printk(KERN_ALTER "page fault addr:%x\n", addr);
        dump_vmspace(cur->vmm);
        print_task();
        trigger_force(TRIGHW, cur->pid);
        /* 发出信号退出 */
        //panic("send a signal SIGSEGV because unknown space!");
        return -1;
    }
    /* 找到空间，判断空间类型，根据不同的空间，进行不同的处理。 */
    if (space->start > addr) { /* 故障地址在空间前，说明是栈向下拓展，那么尝试拓展栈。 */
        if (frame->error_code & PG_ERR_USER) { /* 故障源是用户 */
            /* 可拓展栈：有栈标志，在可拓展限定内， */
            if ((space->flags & VMS_MAP_STACK) &&
                ((space->end - space->start) < MAX_VMS_STACK_SIZE) &&
                (addr + 32 >= frame->esp)) {
                /* 向下扩展栈 */
                do_expand_stack(space, addr);
                //printk(KERN_DEBUG "expand stack at %x\n", addr);
            } else {    /* 不是可拓展栈 */
                printk("task name=%s pid=%d\n", cur->name, cur->pid);
                printk(KERN_ALTER "do_page_fault: touch TRIGHW trigger because unknown space!\n");
                printk(KERN_ALTER "page fault addr:%x\n", addr);
                dump_trap_frame(frame);
                //dump_vmspace(cur->vmm);
        
                trigger_force(TRIGHW, cur->pid);
                /* 发出信号退出 */
                //panic("send a signal SIGSEGV because addr %x without space!", addr); 
                return -1;  
            }    
        }
    }
    /* 故障地址在空间里面，情况如下：
    1.读写保护故障（R/W）
    2.缺少物理页和虚拟地址的映射。（堆的向上拓展或者栈的向下拓展）
     */
    if (frame->error_code & PG_ERR_PROTECT) {
        return do_protection_fault(space, addr, frame->error_code & PG_ERR_WRITE);
    }
    /* 处理缺页 */
    handle_no_page(addr, space->page_prot);
    //printk(KERN_DEBUG "handle_no_page at %x success!\n", addr);
    
    /* 执行完缺页故障处理后，会到达这里表示成功！ */
    return 0;
}
void __page_dir_active(unsigned int page, int on)
{
    unsigned long paddr = PAGE_DIR_PHY_ADDR;
    if (on) {
        paddr = page;
    }
    write_cr3(paddr);
    update_tss_info((unsigned long )current_task);
}