#include <xbook/sharemem.h>
#include <xbook/memcache.h>
#include <xbook/debug.h>
#include <xbook/string.h>
#include <xbook/memops.h>
#include <sys/shm.h>

/* debug shm : 1 enable, 0 disable */
#define DEBUG_SHM 0

share_mem_t *share_mem_table;

/* 通过某种机制来获取到一个共享内存的使用，如果多个进程需要获取到同一个
共享内存，那么就需要有一个共同的标识，而名字是最为合适的。 
共享内存大小总是以页为单位对齐的。
*/

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
 * share_mem_alloc - 分配一个共享内存
 * @name: 名字
 * @size: 大小
 * @flags: 标志
 * 
 * 从共享内存表中分配一个共享内存
 * 
 * @return: 成功返回共享内存结构的地址，失败返回NULL
 */
share_mem_t *share_mem_alloc(char *name, unsigned long size, unsigned long flags)
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
            shm->page_addr = alloc_pages(shm->npages);  /* 分配物理内存 */
            if (!shm->page_addr)    /* 分配失败，返回NULL */
                return NULL;
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
    if (free_pages(shm->page_addr))
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
 *         SHM_CREAT: 如果共享内存不存在，则创建一个新的共享内存，否则就打开
 *         SHM_EXCL:  和CREAT一起使用，则要求创建一个新的共享内存，若已存在，就返回-1。
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
    share_mem_t *shm;
    /* 有创建标志 */
    if (flags & SHM_CREAT) { /* 创建一个新的共享区域 */
        if (flags & SHM_EXCL) { /* 必须不存在才行 */
            craete_new = 1; /* 需要创建一个新的共享内存 */
        }
        shm = share_mem_find_by_name(name);
        if (shm) {  /* 共享内存已经存在 */
            if (craete_new) /* 必须创建一个新的，不能是已经存在的，故错误 */
                return -1;
            
            /* 已经存在，那么就返回已经存在的共享内存的id */
            return shm->id;
        } else { /* 不存在则创建一个新的 */
            shm = share_mem_alloc(name, size, flags);
            return shm->id; /* 返回共享内存id */
        }
    }
    /* 没有创建标志，直接返回错误 */
    return -1;
}


/**
 * share_mem_get - 获取一个共享内存
 * 
 * @name: 共享内存名
 * @size: 共享内存大小
 * @flags: 获取标志
 *         SHM_CREAT: 如果共享内存不存在，则创建一个新的共享内存，否则就打开
 *         SHM_EXCL:  和CREAT一起使用，则要求创建一个新的共享内存，若已存在，就返回-1。
 *                    相当于在CREAT上面加了一个必须不存在的限定。
 * 
 * @return: 成功返回共享区域id，失败返回-1
 */
int share_mem_put(int shmid)
{
    share_mem_t *shm;
    shm = share_mem_find_by_id(shmid);

    if (shm) {  /* 共享内存存在 */
        share_mem_free(shm);
        return 0;
    }
    /* 没有找到共享区域 */
    return -1;
}

/**
 * init_share_mem - 初始化共享内存
 */
void init_share_mem()
{
    share_mem_table = (share_mem_t *)kmalloc(sizeof(share_mem_t) * MAX_SHARE_MEM_NR);
    if (share_mem_table == NULL) /* must be ok! */
        panic(KERN_EMERG "init_share_mem: alloc mem for share_mem_table failed! :(\n");
    printk(KERN_DEBUG "init_share_mem: alloc mem table at %x\n", share_mem_table);   
    int i;
    for (i = 0; i < MAX_SHARE_MEM_NR; i++) {
        share_mem_table[i].id = 1 + i + i * 2; /* 共享内存id */
        share_mem_table[i].page_addr = 0;
        share_mem_table[i].npages = 0;
        memset(share_mem_table[i].name, 0, SHARE_MEM_NAME_LEN);
    }
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
    spin(":)");
}