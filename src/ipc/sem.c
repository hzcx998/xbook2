#include <xbook/memcache.h>
#include <xbook/debug.h>
#include <string.h>
#include <string.h>
#include <xbook/sem.h>
#include <sys/ipc.h>

#define DEBUG_SEM 0

sem_t *sem_table;
DEFINE_SEMAPHORE(sem_mutex, 1);

static sem_t *sem_find_by_name(char *name)
{
    sem_t *sem;
    int i;
    for (i = 0; i < SEM_MAX_NR; i++) {
        sem = &sem_table[i];
        if (sem->name[0] != '\0') {
            if (!strcmp(sem->name, name)) {
                return sem;
            }
        }
    }
    return NULL;
}

static sem_t *sem_find_by_id(int semid)
{
    sem_t *sem;
    int i;
    for (i = 0; i < SEM_MAX_NR; i++) {
        sem = &sem_table[i];
        if (sem->id == semid && sem->name[0] != '\0') { 
            return sem;
        }
    }
    return NULL;
}

sem_t *sem_alloc(char *name, int value)
{
    sem_t *sem;
    int i;
    for (i = 0; i < SEM_MAX_NR; i++) {
        sem = &sem_table[i];
        if (sem->name[0] == '\0') {
            semaphore_init(&sem->sema, value);
            memcpy(sem->name, name, SEM_NAME_LEN);
            sem->name[SEM_NAME_LEN - 1] = '\0';
            return sem;
        }
    }
    return NULL;
}

int sem_free(sem_t *sem)
{
    memset(sem->name, 0, SEM_NAME_LEN);
    return 0;
}

/**
 * @semflg: 获取标志
 *         IPC_CREAT: 如果信号量不存在，则创建一个新的信号量，否则就打开
 *         IPC_EXCL:  和CREAT一起使用，则要求创建一个新的信号量，若已存在，就返回-1。
 *                    相当于在CREAT上面加了一个必须不存在的限定。
 * @return: 成功返回信号量id，失败返回-1
 */
int sem_get(char *name, int value, int semflg)
{
    if (name == NULL)
        return -1;
    char craete_new = 0;
    int retval = -1;
    sem_t *sem;
    semaphore_down(&sem_mutex);
    /* 有创建标志 */
    if (semflg & IPC_CREAT) {
        if (semflg & IPC_EXCL) {
            craete_new = 1;
        }
        sem = sem_find_by_name(name);
        if (sem) {
            if (craete_new)
                goto err;
            printk(KERN_DEBUG "sem_get: find a exist sem %d.\n", sem->id);
            retval = sem->id;
        } else {
            sem = sem_alloc(name, value);
            if (sem == NULL)
                goto err;
            printk(KERN_DEBUG "sem_get: alloc a new sem %d.\n", sem->id);
            retval = sem->id;
        }
    }
err:
    semaphore_up(&sem_mutex);
    return retval;
}

int sem_put(int semid)
{
    sem_t *sem;
    semaphore_down(&sem_mutex);
    sem = sem_find_by_id(semid);
    if (sem) {
#if DEBUG_SEM == 1
        printk(KERN_INFO "sem value %d.\n", atomic_get(&sem->sema.counter));
#endif
        sem_free(sem);
        semaphore_up(&sem_mutex);
        return 0;
    }
    semaphore_up(&sem_mutex);
    return -1;
}

/**
 * @semflg: 操作标志。
 *          IPC_NOWAIT: 如果没有资源，就直接返回-1，不阻塞
 */
int sem_down(int semid, int semflg)
{
    sem_t *sem;
    semaphore_down(&sem_mutex);
    sem = sem_find_by_id(semid);
    semaphore_up(&sem_mutex);
    if (sem == NULL) {
        return -1;
    }
    if (semflg & IPC_NOWAIT) {
        if (semaphore_try_down(&sem->sema))
            return -1;
    } else {
        semaphore_down(&sem->sema);
    }
    return 0;
}

int sem_up(int semid)
{
    sem_t *sem;
    semaphore_down(&sem_mutex);
    sem = sem_find_by_id(semid);
    semaphore_up(&sem_mutex);
    if (sem == NULL) {
        return -1;
    }
    semaphore_up(&sem->sema);
    return 0;
}

int sys_sem_get(char *name, int value, int semflg)
{
    return sem_get(name, value, semflg);
}

int sys_sem_put(int semid)
{
    return sem_put(semid);
}

int sys_sem_down(int semid, int semflg)
{
    return sem_down(semid, semflg);
}

int sys_sem_up(int semid)
{
    return sem_up(semid);
}

void sem_init()
{
    sem_table = (sem_t *)mem_alloc(sizeof(sem_t) * SEM_MAX_NR);
    if (sem_table == NULL)
        panic(KERN_EMERG "sem_init: alloc mem for sem_table failed! :(\n");
    int i;
    for (i = 0; i < SEM_MAX_NR; i++) {
        sem_table[i].id = 1 + i + i * 2;
        semaphore_init(&sem_table[i].sema, 0);
        memset(sem_table[i].name, 0, SEM_NAME_LEN);
    }
}