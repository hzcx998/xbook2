#include <xbook/memcache.h>
#include <xbook/debug.h>
#include <string.h>
#include <string.h>
#include <xbook/sem.h>
#include <sys/ipc.h>

/* debug sem : 1 enable, 0 disable */
#define DEBUG_SEM 0

sem_t *sem_table;

/* 保护信号量的分配与释放 */
DEFINE_SEMAPHORE(sem_mutex, 1);

/**
 * sem_find_by_name - 通过名字查找信号量
 * @name: 信号量的名字
 * 
 * @return: 如果信号量已经在信号量表中，就返回信号量指针，
 *          没有则返回NULL
 */
static sem_t *sem_find_by_name(char *name)
{
    sem_t *sem;
    int i;
    for (i = 0; i < SEM_MAX_NR; i++) {
        sem = &sem_table[i];
        if (sem->name[0] != '\0') { /* 有名字才进行比较 */
            if (!strcmp(sem->name, name)) {
                return sem;
            }
        }
    }
    return NULL;
}


/**
 * sem_find_by_id - 通过id查找信号量
 * @id: 信号量的id
 * 
 * @return: 如果信号量已经在信号量表中，就返回信号量指针，
 *          没有则返回NULL
 */
static sem_t *sem_find_by_id(int semid)
{
    sem_t *sem;
    int i;
    for (i = 0; i < SEM_MAX_NR; i++) {
        sem = &sem_table[i];
        /* id相同并且正在使用，才找到 */
        if (sem->id == semid && sem->name[0] != '\0') { 
            return sem;
        }
    }
    return NULL;
}

/**
 * sem_alloc - 分配一个信号量
 * @name: 名字
 * @size: 大小
 * 
 * 从信号量表中分配一个信号量
 * 
 * @return: 成功返回信号量结构的地址，失败返回NULL
 */
sem_t *sem_alloc(char *name, int value)
{
    sem_t *sem;
    int i;
    for (i = 0; i < SEM_MAX_NR; i++) {
        sem = &sem_table[i];
        if (sem->name[0] == '\0') { /* 没有名字才使用 */
            semaphore_init(&sem->sema, value);
            /* 设置信号量名字 */
            memcpy(sem->name, name, SEM_NAME_LEN);
            sem->name[SEM_NAME_LEN - 1] = '\0';
#if DEBUG_SEM == 1
            printk(KERN_DEBUG "sem_alloc: sem id=%d\n", sem->id);
#endif
            return sem; /* 返回信号量 */
        }
    }
    return NULL;
}

/**
 * sem_free - 释放一个信号量
 * @sem: 信号量
 * 
 * @return: 成功返回0，失败返回-1
 */
int sem_free(sem_t *sem)
{
#if DEBUG_SEM == 1    
    printk(KERN_DEBUG "sem_free: sem id=%d\n",sem->id);
#endif
    memset(sem->name, 0, SEM_NAME_LEN);
    return 0;
}

/**
 * sem_get - 获取一个信号量
 * 
 * @name: 信号量名
 * @value: 信号量大小
 * @semflg: 获取标志
 *         IPC_CREAT: 如果信号量不存在，则创建一个新的信号量，否则就打开
 *         IPC_EXCL:  和CREAT一起使用，则要求创建一个新的信号量，若已存在，就返回-1。
 *                    相当于在CREAT上面加了一个必须不存在的限定。
 * 
 * @return: 成功返回信号量id，失败返回-1
 */
int sem_get(char *name, int value, int semflg)
{
    /* 检测参数 */    
    if (name == NULL)
        return -1;
    char craete_new = 0; /* 是否需要创建一个新的信号量 */
    int retval = -1;
    sem_t *sem;
    semaphore_down(&sem_mutex);
    /* 有创建标志 */
    if (semflg & IPC_CREAT) { /* 创建一个新的信号量 */
        if (semflg & IPC_EXCL) { /* 必须不存在才行 */
            craete_new = 1; /* 需要创建一个新的信号量 */
        }
        
        sem = sem_find_by_name(name);
        if (sem) {  /* 信号量已经存在 */
            if (craete_new) /* 必须创建一个新的，不能是已经存在的，故错误 */
                goto err;
            printk(KERN_DEBUG "sem_get: find a exist sem %d.\n", sem->id);
            /* 已经存在，那么就返回已经存在的信号量的id */
            retval = sem->id;
        } else { /* 不存在则创建一个新的 */
            
            sem = sem_alloc(name, value);
            if (sem == NULL)
                goto err;
            
            printk(KERN_DEBUG "sem_get: alloc a new sem %d.\n", sem->id);
            retval = sem->id; /* 返回信号量id */
        }
    }
err:
    semaphore_up(&sem_mutex);
    /* 没有创建标志，直接返回错误 */
    return retval;
}

/**
 * sem_put - 释放一个信号量
 * 
 * @semid: 信号量id
 * 
 * @return: 成功返回0，失败返回-1
 */
int sem_put(int semid)
{
    sem_t *sem;
    semaphore_down(&sem_mutex);
    sem = sem_find_by_id(semid);
    
    if (sem) {  /* 信号量存在 */
#if DEBUG_SEM == 1
        printk(KERN_INFO "sem value %d.\n", atomic_get(&sem->sema.counter));
#endif
        sem_free(sem);
        semaphore_up(&sem_mutex);
        return 0;
    }
    semaphore_up(&sem_mutex);
    /* 没有找到信号量 */
    return -1;
}

/**
 * sem_down - 减少信号量
 * @semid: 信号量的id
 * @semflg: 操作标志。
 *          IPC_NOWAIT: 如果没有资源，就直接返回-1，不阻塞
 * 
 * @return: 成功返回0，失败返回-1
 */
int sem_down(int semid, int semflg)
{
    sem_t *sem;
    semaphore_down(&sem_mutex);
    sem = sem_find_by_id(semid);
    semaphore_up(&sem_mutex);
    if (sem == NULL) { /* not found sem */
        return -1;
    }
    if (semflg & IPC_NOWAIT) { /* 尝试down */
        if (semaphore_try_down(&sem->sema))
            return -1;
    } else {    /* 一般的down */
        semaphore_down(&sem->sema);
    }
    return 0;
}

/**
 * sem_up - 增加信号量
 * @semid: 信号量的id
 * 
 * @return: 成功返回0，失败返回-1
 */
int sem_up(int semid)
{
    sem_t *sem;
    semaphore_down(&sem_mutex);
    sem = sem_find_by_id(semid);
    semaphore_up(&sem_mutex);
    if (sem == NULL) { /* not found sem */
        return -1;
    }
    semaphore_up(&sem->sema);
    return 0;
}

/**
 * init_sem - 初始化信号量
 */
void init_sem()
{
    sem_table = (sem_t *)kmalloc(sizeof(sem_t) * SEM_MAX_NR);
    if (sem_table == NULL) /* must be ok! */
        panic(KERN_EMERG "init_sem: alloc mem for sem_table failed! :(\n");
    //printk(KERN_DEBUG "init_sem: alloc mem table at %x\n", sem_table);   
    int i;
    for (i = 0; i < SEM_MAX_NR; i++) {
        sem_table[i].id = 1 + i + i * 2; /* 信号量id */
        semaphore_init(&sem_table[i].sema, 0);
        memset(sem_table[i].name, 0, SEM_NAME_LEN);
    }
#if 0
    int semid = sem_get("test", 1, IPC_CREAT);
    if (semid == -1)
        printk(KERN_ERR "get sem failed!\n");
    printk(KERN_DEBUG "get sem %d.\n", semid);
    semid = sem_get("test", 1, IPC_CREAT);
    if (semid == -1)
        printk(KERN_ERR "get sem failed!\n");
    printk(KERN_DEBUG "get sem %d.\n", semid);
    sem_put(semid);
    semid = sem_get("test", 1, IPC_CREAT | IPC_EXCL);
    if (semid == -1)
        printk(KERN_ERR "get sem failed!\n");
    printk(KERN_DEBUG "get sem %d.\n", semid);
    
#endif 
    // spin(":)");
}