#ifndef COND_SYNC_H
#define COND_SYNC_H
 
 
#include<pthread/pthread.h>
 
typedef struct Cond_sync_tag
{
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int sig_num; //signal's num
    int valid;
}Cond_sync_t;
 
 
#define COND_SYNC_VALID 0xabcd
 
 
#define COND_SYNC_INITIALIZER_V(num) \
{ PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, num, COND_SYNC_VALID}
 
#define COND_SYNC_INITIALIZER COND_SYNC_INITIALIZER_V(0)
 
int cond_sync_init(Cond_sync_t* cond_s);
int cond_sync_destroy(Cond_sync_t* cond_s);
int cond_sync_wait(Cond_sync_t* cond_s);
int cond_sync_timedwait(Cond_sync_t* cond_s, int msecs); //Millisecond
int cond_sync_signal(Cond_sync_t* cond_s);
 
#endif // COND_SYNC_H
