#ifndef _XBOOK_SEM_H
#define _XBOOK_SEM_H

#include "semaphore.h"

#define SEM_MAX_NR        128
#define SEM_MAX_VALUE     (‭2147483647‬)
#define SEM_NAME_LEN      24

typedef struct {
    unsigned short id;
    semaphore_t sema;           /* 内核信号量，用户信号量是对内核信号量的封装 */
    char name[SEM_NAME_LEN];    
} sem_t;

sem_t *sem_alloc(char *name, int value);
int sem_free(sem_t *shm);

int sem_get(char *name, int value, int semflg);
int sem_put(int semid);


int sem_down(int semid, int semflg);
int sem_up(int semid);

void sem_init();

int sys_sem_get(char *name, int value, int semflg);
int sys_sem_put(int semid);
int sys_sem_down(int semid, int semflg);
int sys_sem_up(int semid);

#endif   /* _XBOOK_SEM_H */
