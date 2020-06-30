
#ifndef _XLIBC_SEMAPHORE_H
#define _XLIBC_SEMAPHORE_H

#include <sys/time.h>
#include <pthread.h>

/* Maximum value the semaphore can have.  */
#define SEM_VALUE_MAX   (2147483647)


/* 信号量结构体 */
typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int value; //signal's num
    int valid;
} sem_t;

#define SEM_VALID (0x19980325)

#define SEM_INITIALIZER_V(val) \
{ PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, val, COND_SYNC_VALID}
 
#define SEM_INITIALIZER SEM_INITIALIZER_V(0)
 
int sem_init(sem_t *sem, int pshared, unsigned int value);
int sem_destroy(sem_t *sem);
int sem_getvalue(sem_t *sem, int *sval);
int sem_wait(sem_t *sem);
int sem_trywait(sem_t *sem);
int sem_timedwait(sem_t *sem, const struct timespec *abs_timeout);
int sem_timedwait2(sem_t* sem, int msecs);
int sem_post(sem_t *sem);

#endif  /* _XLIBC_SEMAPHORE_H */
