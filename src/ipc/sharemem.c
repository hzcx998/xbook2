#include <xbook/sharemem.h>
#include <xbook/memcache.h>
#include <xbook/debug.h>
#include <xbook/safety.h>
#include <string.h>
#include <string.h>
#include <xbook/memspace.h>
#include <xbook/semaphore.h>
#include <xbook/task.h>
#include <xbook/schedule.h>
#include <sys/ipc.h>
#include <errno.h>

share_mem_t *share_mem_table;
DEFINE_SEMAPHORE(share_mem_mutex, 1);

static share_mem_t *share_mem_find_by_name(char *name)
{
    share_mem_t *shm;
    int i;
    for (i = 0; i < MAX_SHARE_MEM_NR; i++) {
        shm = &share_mem_table[i];
        if (shm->name[0] != '\0') {
            if (!strcmp(shm->name, name)) {
                return shm;
            }
        }
    }
    return NULL;
}

static share_mem_t *share_mem_find_by_id(int shmid)
{
    share_mem_t *shm;
    int i;
    for (i = 0; i < MAX_SHARE_MEM_NR; i++) {
        shm = &share_mem_table[i];
        if (shm->id == shmid && shm->name[0] != '\0') { 
            return shm;
        }
    }
    return NULL;
}

share_mem_t *share_mem_find_by_addr(addr_t addr)
{
    share_mem_t *shm;
    int i;
    for (i = 0; i < MAX_SHARE_MEM_NR; i++) {
        shm = &share_mem_table[i];
        if (shm->name[0] != '\0') {
            if (shm->page_addr == addr) { 
                return shm;
            }
        }
        
    }
    return NULL;
}

share_mem_t *share_mem_alloc(char *name, unsigned long size)
{
    share_mem_t *shm;
    int i;
    for (i = 0; i < MAX_SHARE_MEM_NR; i++) {
        shm = &share_mem_table[i];
        if (shm->name[0] == '\0') {
            if (!size)
                size = 1;
            size = PAGE_ALIGN(size);
            shm->npages = size / PAGE_SIZE;
            shm->page_addr = 0;
            shm->flags = 0;
            memcpy(shm->name, name, SHARE_MEM_NAME_LEN);
            shm->name[SHARE_MEM_NAME_LEN - 1] = '\0';
            return shm;
        }
    }
    return NULL;
}

int share_mem_free(share_mem_t *shm)
{
    if (shm->page_addr && !(shm->flags & SHARE_MEM_PRIVATE))
        if (page_free(shm->page_addr))
            return -1;
    memset(shm->name, 0, SHARE_MEM_NAME_LEN);
    return 0;
}

/**
 * @flags: 获取标志
 *         IPC_CREAT: 如果共享内存不存在，则创建一个新的共享内存，否则就打开
 *         IPC_EXCL:  和CREAT一起使用，则要求创建一个新的共享内存，若已存在，就返回-1。
 *                    相当于在CREAT上面加了一个必须不存在的限定。
 * @return: 成功返回共享区域id，失败返回-1
 */
int share_mem_get(char *name, unsigned long size, unsigned long flags)
{
    if (name == NULL)
        return -1;
    if (size > 0 && PAGE_ALIGN(size) >= MAX_SHARE_MEM_SIZE)
        return -1;
    char craete_new = 0;
    int retval = -1;
    share_mem_t *shm;
    semaphore_down(&share_mem_mutex);
    if (flags & IPC_CREAT) {
        if (flags & IPC_EXCL) {
            craete_new = 1;
        }
        shm = share_mem_find_by_name(name);
        if (shm) {
            if (craete_new)
                goto err;
            retval = shm->id;
        } else {
            shm = share_mem_alloc(name, size);
            if (shm == NULL)
                goto err;
            retval = shm->id;
        }
    }
err:
    semaphore_up(&share_mem_mutex);
    return retval;
}

/**
 * @shmaddr: 共享内存的地址
 *          若该参数为NULL，则在进程空间自动选择一个闲的地址来映射，
 *          不为空，那么就在进程空间映射为该地址
 * 把共享内存的物理地址映射到当前进程的进程空间，
 * 需要用到的映射是虚拟地址和物理地址直接映射，不需要分配物理页，
 * 因为已经在分配共享内存时分配了物理页。
 * @return: 成功返回映射在进程空间的地址，失败返回-1
 */
void *share_mem_map(int shmid, void *shmaddr, int shmflg)
{
    share_mem_t *shm;
    semaphore_down(&share_mem_mutex);
    shm = share_mem_find_by_id(shmid);
    semaphore_up(&share_mem_mutex);
    if (shm == NULL) {
        errprint("shm %d not fouded!" endl, shmid);
        return (void *) -1;
    }   
    task_t *cur = task_current;
    unsigned long addr;
    unsigned long len = shm->npages * PAGE_SIZE;
    if (shmaddr == NULL) {
        addr = mem_space_get_unmaped(cur->vmm, shm->npages * PAGE_SIZE);
        if (addr == -1) {
            return (void *) -1;
        }
        if (addr < cur->vmm->map_start || 
            addr + len >= cur->vmm->map_end)
            return (void *) -1;
        if (mem_space_find_intersection(cur->vmm, addr, addr + len))
            return (void *) -1;
        if (!shm->page_addr) {
            shm->page_addr = page_alloc_user(shm->npages);
            if (!shm->page_addr)
                return (void *) -1;
        }
        unsigned long flags = MEM_SPACE_MAP_FIXED | MEM_SPACE_MAP_SHARED;
        if (shmflg & IPC_REMAP) {
            flags |= MEM_SPACE_MAP_REMAP;
        }
        shmaddr = mem_space_mmap(addr, shm->page_addr, shm->npages * PAGE_SIZE,
            PROT_USER | PROT_WRITE, flags);
        
    } else {
        unsigned long vaddr;
        if (shmflg & IPC_RND)
            vaddr = (unsigned long) shmaddr & PAGE_MASK;
        else 
            vaddr = (unsigned long) shmaddr;
        if (!shm->page_addr) {
            shm->page_addr = addr_vir2phy(vaddr);
            if (!shm->page_addr)
                return (void *) -1;
            shm->flags |= SHARE_MEM_PRIVATE;
        }
        shmaddr = (void *)vaddr;
    }
    if (shmaddr != (void *) -1)
        atomic_inc(&shm->links);
    return shmaddr;
}

int share_mem_unmap(const void *shmaddr, int shmflg)
{
    if (!shmaddr) {
        return -1;
    }
    task_t *cur = task_current;
    unsigned long addr;
    if (shmflg & IPC_RND)
        addr = (unsigned long) shmaddr & PAGE_MASK;
    else 
        addr = (unsigned long) shmaddr;
    mem_space_t *sp = mem_space_find(cur->vmm, addr);
    if (sp == NULL) {
        kprint(PRINT_DEBUG "share_mem_unmap: not fond space\n");
        return -1;
    }
    addr = addr_vir2phy(addr);
    semaphore_down(&share_mem_mutex);
    share_mem_t *shm = share_mem_find_by_addr(addr);
    semaphore_up(&share_mem_mutex);
    
    int retval = 0;
    if (!(shm->flags & SHARE_MEM_PRIVATE)) {
        retval = do_mem_space_unmap(cur->vmm, sp->start, sp->end - sp->start);
    }
    if (retval != -1) {
        if (shm) {
            atomic_dec(&shm->links);
        }
    } else {
        kprint(PRINT_ERR "share_mem_unmap: do unmap at %x failed!\n", addr);
    }
    return retval;
}

int share_mem_put(int shmid)
{
    share_mem_t *shm;
    semaphore_down(&share_mem_mutex);
    shm = share_mem_find_by_id(shmid);
    if (shm) {  
        if (atomic_get(&shm->links) <= 0) {
            share_mem_free(shm);
        } 
        semaphore_up(&share_mem_mutex);
        return 0;
    }
    semaphore_up(&share_mem_mutex);
    return -1;
}

int share_mem_inc(int shmid)
{
    share_mem_t *shm;
    semaphore_down(&share_mem_mutex);
    shm = share_mem_find_by_id(shmid);
    if (shm) {
        atomic_inc(&shm->links);
        semaphore_up(&share_mem_mutex);
        return 0;
    }
    semaphore_up(&share_mem_mutex);
    return -1;
}

int share_mem_dec(int shmid)
{
    share_mem_t *shm;
    semaphore_down(&share_mem_mutex);
    shm = share_mem_find_by_id(shmid);
    if (shm) {
        atomic_dec(&shm->links);
        semaphore_up(&share_mem_mutex);
        return 0;
    }
    semaphore_up(&share_mem_mutex);
    return -1;
}

int sys_shmem_get(char *name, unsigned long size, unsigned long flags)
{
    if (!name)
        return -EINVAL;
    if (mem_copy_from_user(NULL, name, SHARE_MEM_NAME_LEN) < 0)
        return -EINVAL;
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
    /*if (mem_copy_from_user(NULL, (void *)shmaddr, PAGE_SIZE) < 0)
        return -EINVAL;*/
    return share_mem_unmap(shmaddr, shmflg);
}

void share_mem_init()
{
    share_mem_table = (share_mem_t *)mem_alloc(sizeof(share_mem_t) * MAX_SHARE_MEM_NR);
    if (share_mem_table == NULL) /* must be ok! */
        panic(PRINT_EMERG "share_mem_init: alloc mem for share_mem_table failed! :(\n");
    int i;
    for (i = 0; i < MAX_SHARE_MEM_NR; i++) {
        share_mem_table[i].id = 1 + i + i * 2; /* 共享内存id */
        share_mem_table[i].page_addr = 0;
        share_mem_table[i].npages = 0;
        share_mem_table[i].flags = 0;
        atomic_set(&share_mem_table[i].links, 0);
        memset(share_mem_table[i].name, 0, SHARE_MEM_NAME_LEN);
    }
}
