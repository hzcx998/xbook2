#ifndef _XBOOK_SEM_H
#define _XBOOK_SEM_H

/* 用户可以使用的信号量 */
#include "semaphore.h"

/* 最多有多少个信号量 */
#define SEM_MAX_NR        128

/* 信号量的最大值, int 型数据最大值 */
#define SEM_MAX_VALUE     (‭2147483647‬)

#define SEM_NAME_LEN      24

/* 信号量结构 */
typedef struct {
    unsigned short id;          /* 信号量id */
    semaphore_t sema;           /* 内核信号量，用户信号量是对内核信号量的封装 */
    char name[SEM_NAME_LEN];   /* 名字 */
} sem_t;

sem_t *sem_alloc(char *name, int value);
int sem_free(sem_t *shm);

int sem_get(char *name, int value, int semflg);
int sem_put(int semid);


int sem_down(int semid, int semflg);
int sem_up(int semid);

void init_sem();

int sys_sem_get(char *name, int value, int semflg);
int sys_sem_put(int semid);
int sys_sem_down(int semid, int semflg);
int sys_sem_up(int semid);

#endif   /* _XBOOK_SEM_H */
