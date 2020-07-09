
#ifndef COND_SEM_H
#define COND_SEM_H
 
#include"cond_sync.h"
#include  <errno.h>
 
typedef Cond_sync_t cond_sem_t;
 
#define COND_SEM_INITIALIZER(num) COND_SYNC_INITIALIZER_V(num)
 
 
inline int cond_sem_init(cond_sem_t* con, int num)
{
    int status = cond_sync_init(con);
    con->sig_num = num;
    return status;
}
 
 
inline int cond_sem_p(cond_sem_t* con)
{
    return cond_sync_wait(con);
}
 
 
inline int cond_sem_tryP(cond_sem_t* con)
{
    int status = cond_sync_timedwait(con, 0);
    if( status == ETIMEDOUT )
        return EAGAIN;
	
    return status;
}
 
 
inline int cond_sem_v(cond_sem_t* con)
{
    return cond_sync_signal(con);
}
 
 
inline int cond_sem_destroy(cond_sem_t* con)
{
    return cond_sync_destroy(con);
}
 
 
#endif // COND_SEM_HPP
