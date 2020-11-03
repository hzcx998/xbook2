#include <xbook/sharemem.h>
#include <xbook/memcache.h>
#include <xbook/debug.h>
#include <string.h>
#include <string.h>
#include <xbook/memspace.h>
#include <xbook/semaphore.h>
#include <sys/ipc.h>

/* debug shm : 1 enable, 0 disable */
#define DEBUG_SHM 0

share_mem_t *share_mem_table;

/* 保护共享内存的分配与释放 */
DEFINE_SEMAPHORE(share_mem_mutex, 1);

/**
 * share_mem_find_by_name - 通过名字查找共享内存
 * @name: 共享内存的名字
 * 
 * @return: 如果共享内存已经在共享内存表中，就返回共享内存指针，
 *          没有则返回NULL
 */
static share_mem_t *share_mem_find_by_name(char *name)
{
    share_mem_t *shm;
    int i;
    for (i = 0; i < MAX_SHARE_MEM_NR; i++) {
        shm = &share_mem_table[i];
        if (shm->name[0] != '\0') { /* 有名字才进行比较 */
            if (!strcmp(shm->name, name)) {
                return shm;
            }
        }
    }
    return NULL;
}


/**
 * share_mem_find_by_id - 通过id查找共享内存
 * @id: 共享内存的id
 * 
 * @return: 如果共享内存已经在共享内存表中，就返回共享内存指针，
 *          没有则返回NULL
 */
static share_mem_t *share_mem_find_by_id(int shmid)
{
    share_mem_t *shm;
    int i;
    for (i = 0; i < MAX_SHARE_MEM_NR; i++) {
        shm = &share_mem_table[i];
        /* id相同并且正在使用，才找到 */
        if (shm->id == shmid && shm->name[0] != '\0') { 
            return shm;
        }
    }
    return NULL;
}

/**
 * share_mem_find_by_addr - 通过物理地址查找共享内存
 * @id: 共享内存的物理地址
 * 
 * @return: 如果共享内存已经在共享内存表中，就返回共享内存指针，
 *          没有则返回NULL
 */
share_mem_t *share_mem_find_by_addr(addr_t addr)
{
    share_mem_t *shm;
    int i;
    for (i = 0; i < MAX_SHARE_MEM_NR; i++) {
        shm = &share_mem_table[i];
        if (shm->name[0] != '\0') {
            /* id相同并且正在使用，才找到 */
            if (shm->page_addr == addr) { 
                return shm;
            }
        }
        
    }
    return NULL;
}

/**
 * share_mem_alloc - 分配一个共享内存
 * @name: 名字
 * @size: 大小
 * 
 * 从共享内存表中分配一个共享内存
 * 
 * @return: 成功返回共享内存结构的地址，失败返回NULL
 */
share_mem_t *share_mem_alloc(char *name, unsigned long size)
{
    share_mem_t *shm;
    int i;
    for (i = 0; i < MAX_SHARE_MEM_NR; i++) {
        shm = &share_mem_table[i];
        if (shm->name[0] == '\0') { /* 没有名字才使用 */
            if (!size)  /* 如果大小为0，就设置1，后面会与页面对齐 */
                size = 1;
            size = PAGE_ALIGN(size);
            shm->npages = size / PAGE_SIZE; /* 计算占用的页面数 */
            shm->page_addr = 0;  /* 还没有物理地址 */
            shm->flags = 0;

            /* 设置共享内存名字 */
            memcpy(shm->name, name, SHARE_MEM_NAME_LEN);
            shm->name[SHARE_MEM_NAME_LEN - 1] = '\0';
#if DEBUG_SHM == 1
            printk(KERN_DEBUG "share_mem_alloc: shm id=%d page=%x pages=%d\n",
                shm->id, shm->page_addr, shm->npages);
#endif
            return shm; /* 返回共享内存 */
        }
    }
    return NULL;
}

/**
 * share_mem_free - 释放一个共享内存
 * @shm: 共享内存
 * 
 * @return: 成功返回0，失败返回-1
 */
int share_mem_free(share_mem_t *shm)
{
#if DEBUG_SHM == 1    
    printk(KERN_DEBUG "share_mem_free: shm id=%d page=%x pages=%d\n",
        shm->id, shm->page_addr, shm->npages);
#endif
    /* 要有物理地址并且还不能是映射的当前进程的共享内存 */
    if (shm->page_addr && !(shm->flags & SHARE_MEM_PRIVATE))
        if (page_free(shm->page_addr))
            return -1;
    memset(shm->name, 0, SHARE_MEM_NAME_LEN);

    return 0;
}

/**
 * share_mem_get - 获取一个共享内存
 * 
 * @name: 共享内存名
 * @size: 共享内存大小
 * @flags: 获取标志
 *         IPC_CREAT: 如果共享内存不存在，则创建一个新的共享内存，否则就打开
 *         IPC_EXCL:  和CREAT一起使用，则要求创建一个新的共享内存，若已存在，就返回-1。
 *                    相当于在CREAT上面加了一个必须不存在的限定。
 * 
 * @return: 成功返回共享区域id，失败返回-1
 */
int share_mem_get(char *name, unsigned long size, unsigned long flags)
{
    /* 检测参数 */    
    if (name == NULL)
        return -1;
    if (size > 0 && PAGE_ALIGN(size) >= MAX_SHARE_MEM_SIZE)
        return -1;
    char craete_new = 0; /* 是否需要创建一个新的共享内存 */
    int retval = -1;
    share_mem_t *shm;
    semaphore_down(&share_mem_mutex);
    /* 有创建标志 */
    if (flags & IPC_CREAT) { /* 创建一个新的共享区域 */
        if (flags & IPC_EXCL) { /* 必须不存在才行 */
            craete_new = 1; /* 需要创建一个新的共享内存 */
        }
        
        shm = share_mem_find_by_name(name);
        if (shm) {  /* 共享内存已经存在 */
            if (craete_new) /* 必须创建一个新的，不能是已经存在的，故错误 */
                goto err;
            /* 已经存在，那么就返回已经存在的共享内存的id */
            retval = shm->id;
        } else { /* 不存在则创建一个新的 */
            
            shm = share_mem_alloc(name, size);
            if (shm == NULL)
                goto err;
            retval = shm->id; /* 返回共享内存id */
        }
    }
err:
    semaphore_up(&share_mem_mutex);
    /* 没有创建标志，直接返回错误 */
    return retval;
}

/**
 * share_mem_map - 将共享内存映射到进程空间
 * @shmid: 共享内存的id
 * @shmaddr: 共享内存的地址
 *          若该参数为NULL，则在进程空间自动选择一个闲的地址来映射，
 *          不为空，那么就在进程空间映射为该地址
 * 
 * 把共享内存的物理地址映射到当前进程的进程空间，
 * 需要用到的映射是虚拟地址和物理地址直接映射，不需要分配物理页，
 * 因为已经在分配共享内存时分配了物理页。
 * 
 * @return: 成功返回映射在进程空间的地址，失败返回-1
 */
void *share_mem_map(int shmid, void *shmaddr, int shmflg)
{
    share_mem_t *shm;
    semaphore_down(&share_mem_mutex);
    shm = share_mem_find_by_id(shmid);
    semaphore_up(&share_mem_mutex);
    if (shm == NULL) { /* not found share mem */
        return (void *) -1;
    }   
    task_t *cur = current_task;
    
    unsigned long addr;
    unsigned long len = shm->npages * PAGE_SIZE;

    /* 现在已经找到了共享内存，需要将它映射到当前进程的空间 */
    if (shmaddr == NULL) {  /* 自动选择一个映射地址 */
        /* 获取一个未使用的虚拟地址 */
        addr = mem_space_get_unmaped(cur->vmm, shm->npages * PAGE_SIZE);
        if (addr == -1) /* 已经没有空闲的空间 */
            return (void *) -1;

        /* 检测虚拟地址是否合法 */
        if (addr < cur->vmm->map_start || 
            addr + len >= cur->vmm->map_end) /* 指定地址不在映射范围内 */
            return (void *) -1;
        
        /* 如果有空间和它相交，就返回错误 */
        if (mem_space_find_intersection(cur->vmm, addr, addr + len))
            return (void *) -1;
            
        /* 如果没有需要分配一个新的物理地址，并映射之 */
        if (!shm->page_addr) {
            shm->page_addr = page_alloc_normal(shm->npages);  /* 分配物理内存 */
            if (!shm->page_addr)    /* 分配失败，返回NULL */
                return (void *) -1;
        } 
#if DEBUG_SHM == 1
        printk(KERN_DEBUG "%s: virtual addr:%x physical addr:%x\n", __func__, addr, shm->page_addr);
#endif
        unsigned long flags = MEM_SPACE_MAP_FIXED | MEM_SPACE_MAP_SHARED;
        if (shmflg & IPC_REMAP) {
            flags |= MEM_SPACE_MAP_REMAP;
        }
        /* 把虚拟地址和物理地址进行映射，物理地址是共享的。由于已经确切获取了一个地址，
        所以这里就用固定映射，因为是共享内存，所以使用共享的方式。 */
        shmaddr = mem_space_mmap(addr, shm->page_addr, shm->npages * PAGE_SIZE,
            PROT_USER | PROT_WRITE, flags);
    } else {    /* 把给定的虚拟地址映射成共享内存 */
        unsigned long vaddr;

        if (shmflg & IPC_RND)
            vaddr = (unsigned long) shmaddr & PAGE_MASK; /* 页地址对齐 */
        else 
            vaddr = (unsigned long) shmaddr;
#if DEBUG_SHM == 1
        printk(KERN_DEBUG "%s: old virtual addr:%x\n", __func__, vaddr);
#endif
        /* 映射一个已经存在的物理地址 */
        if (!shm->page_addr) {
            shm->page_addr = addr_vir2phy(vaddr);    /* 直接获取物理地址 */
#if DEBUG_SHM == 1            
            printk(KERN_DEBUG "%s: phy addr:%x.\n", __func__, shm->page_addr);
#endif
            if (!shm->page_addr)    /* 虚拟地址没有映射过 */
                return (void *) -1;
            
            /* 映射本进程中的共享内存 */
            shm->flags |= SHARE_MEM_PRIVATE;
        }
        shmaddr = (void *)vaddr;
    }
#if DEBUG_SHM == 1
    printk(KERN_DEBUG "%s: virtual addr:%x physical addr:%x\n", __func__, addr, shm->page_addr);
#endif
    
    if (shmaddr != (void *) -1)
        atomic_inc(&shm->links);

#if DEBUG_SHM == 1
    printk(KERN_DEBUG "share_mem_map: map at %x\n", shmaddr);
#endif
    return shmaddr;
}

/**
 * share_mem_unmap - 取消共享内存映射在进程空间中的映射
 * @shmaddr: 共享内存地址
 * 
 * @return: 成功返回0，失败返回-1
 */
int share_mem_unmap(const void *shmaddr, int shmflg)
{
    if (!shmaddr) {
        return -1;
    }
    task_t *cur = current_task;

    unsigned long addr;
    if (shmflg & IPC_RND)
        addr = (unsigned long) shmaddr & PAGE_MASK; /* 页地址对齐 */
    else 
        addr = (unsigned long) shmaddr;

    mem_space_t *sp = mem_space_find(cur->vmm, addr);
    if (sp == NULL) {/* 没有找到对应的空间 */
        printk(KERN_DEBUG "share_mem_unmap: not fond space\n");
        return -1;
    }
    
    addr = addr_vir2phy(addr); /* 通过用户虚拟地址获取物理地址 */
#if DEBUG_SHM == 1
    printk(KERN_DEBUG "%s: shmaddr :%x physical addr:%x\n", __func__, shmaddr, addr);
#endif
    semaphore_down(&share_mem_mutex);
    share_mem_t *shm = share_mem_find_by_addr(addr);
    semaphore_up(&share_mem_mutex);
    int retval = 0;
    /* 不是映射已经存在的内存区域才会取消映射 */
    if (!(shm->flags & SHARE_MEM_PRIVATE)) {
        /* 取消虚拟空间映射 */
        retval = do_mem_space_unmap(cur->vmm, sp->start, sp->end - sp->start);
    }
    if (retval != -1) {  /* 减少链接数 */
        if (shm) {
            #if DEBUG_SHM == 1
            pr_dbg("[shm]: shm addr=%x links=%d.\n", addr, atomic_get(&shm->links));
            #endif
            atomic_dec(&shm->links);
        }
         
    } else {
        printk(KERN_ERR "share_mem_unmap: do unmap at %x failed!\n", addr);
    }
#if DEBUG_SHM == 1
    printk(KERN_DEBUG "share_mem_unmap: unmap at %x\n", addr);
#endif
    return retval;
}

/**
 * share_mem_put - 释放一个共享内存
 * 
 * @shmid: 共享内存id
 * 
 * @return: 成功返回0，失败返回-1
 */
int share_mem_put(int shmid)
{
    share_mem_t *shm;
    semaphore_down(&share_mem_mutex);
    shm = share_mem_find_by_id(shmid);
    /* 共享内存存在，并且链接次数为0才真正释放 */
    if (shm) {  
#if DEBUG_SHM == 1
        printk(KERN_INFO "shm links %d.\n", atomic_get(&shm->links));
#endif
        if (atomic_get(&shm->links) <= 0) {
            
            #if DEBUG_SHM == 1
            pr_dbg("[shm]: destroy shm %d, links %d\n", shmid, atomic_get(&shm->links));
            #endif
            share_mem_free(shm);
        }
            
        semaphore_up(&share_mem_mutex);
        return 0;
    }
    semaphore_up(&share_mem_mutex);
    /* 没有找到共享区域 */
    return -1;
}

/**
 * init_share_mem - 初始化共享内存
 */
void init_share_mem()
{
    share_mem_table = (share_mem_t *)mem_alloc(sizeof(share_mem_t) * MAX_SHARE_MEM_NR);
    if (share_mem_table == NULL) /* must be ok! */
        panic(KERN_EMERG "init_share_mem: alloc mem for share_mem_table failed! :(\n");
    //printk(KERN_DEBUG "init_share_mem: alloc mem table at %x\n", share_mem_table);   
    int i;
    for (i = 0; i < MAX_SHARE_MEM_NR; i++) {
        share_mem_table[i].id = 1 + i + i * 2; /* 共享内存id */
        share_mem_table[i].page_addr = 0;
        share_mem_table[i].npages = 0;
        share_mem_table[i].flags = 0;
        atomic_set(&share_mem_table[i].links, 0);
        memset(share_mem_table[i].name, 0, SHARE_MEM_NAME_LEN);
    }
#if 0
    int shmid = share_mem_get("test", 1, SHM_CREAT);
    if (shmid == -1)
        printk(KERN_ERR "get shm failed!\n");
    printk(KERN_DEBUG "get shm %d.\n", shmid);
    shmid = share_mem_get("test", 1, SHM_CREAT);
    if (shmid == -1)
        printk(KERN_ERR "get shm failed!\n");
    printk(KERN_DEBUG "get shm %d.\n", shmid);
    shmid = share_mem_get("test", 1, SHM_CREAT | SHM_EXCL);
    if (shmid == -1)
        printk(KERN_ERR "get shm failed!\n");
    printk(KERN_DEBUG "get shm %d.\n", shmid);
    
    shmid = share_mem_get("test", MAX_SHARE_MEM_SIZE, SHM_CREAT | SHM_EXCL);
    if (shmid == -1)
        printk(KERN_ERR "get shm failed!\n");
    printk(KERN_DEBUG "get shm %d.\n", shmid);
    
    shmid = share_mem_get("test2", PAGE_SIZE * 19, SHM_CREAT | SHM_EXCL);
    if (shmid == -1)
        printk(KERN_ERR "get shm failed!\n");
    printk(KERN_DEBUG "get shm %d.\n", shmid);
    
    if (share_mem_put(shmid)) {
        printk(KERN_ERR "put shm failed!\n");    
    }
    if (share_mem_put(shmid)) {
        printk(KERN_ERR "put shm failed!\n");    
    }
    shmid = share_mem_get("test", 0, SHM_CREAT);
    if (shmid == -1)
        printk(KERN_ERR "get shm failed!\n");
    printk(KERN_DEBUG "get shm %d.\n", shmid);
    if (share_mem_put(shmid)) {
        printk(KERN_ERR "put shm failed!\n");    
    }
#endif 
    // spin(":)");
}

/**
 * share_mem_grow - 增长计数
 * 
 * @shmid: 共享内存id
 * 
 * @return: 成功返回0，失败返回-1
 */
int share_mem_grow(int shmid)
{
    share_mem_t *shm;
    semaphore_down(&share_mem_mutex);
    shm = share_mem_find_by_id(shmid);
    if (shm) {
        #ifdef DEBUG_IPC_SHM
        pr_dbg("[shm]: shmid=%d before grow links=%d.\n", shmid, atomic_get(&shm->links));
        #endif
        atomic_inc(&shm->links);
        #ifdef DEBUG_IPC_SHM
        pr_dbg("[shm]: shmid=%d after grow links=%d.\n", shmid, atomic_get(&shm->links));
        #endif
        semaphore_up(&share_mem_mutex);
        return 0;
    }
    semaphore_up(&share_mem_mutex);
    /* 没有找到共享区域 */
    return -1;
}

int sys_shmem_get(char *name, unsigned long size, unsigned long flags)
{
    return share_mem_get(name, size, flags);
}

int sys_shmem_put(int shmid)
{
    return share_mem_put(shmid);
}

void *sys_shmem_map(int shmid, void *shmaddr, int shmflg)
{
    return share_mem_map(shmid, shmaddr, shmflg);
}

int sys_shmem_unmap(const void *shmaddr, int shmflg)
{
    return share_mem_unmap(shmaddr, shmflg);
}