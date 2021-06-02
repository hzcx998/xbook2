#ifndef _SYS_SELECT_H
#define _SYS_SELECT_H

#include <sys/time.h>

// #define DEBUG_SELECT

#define FD_SETSIZE    64
#define FD_SET(n, p)  ((p)->fd_bits[(n)/8] |=  (1 << ((n) & 7)))
#define FD_CLR(n, p)  ((p)->fd_bits[(n)/8] &= ~(1 << ((n) & 7)))
#define FD_ISSET(n,p) ((p)->fd_bits[(n)/8] &   (1 << ((n) & 7)))
#define FD_ZERO(p)    memset((void*)(p),0,sizeof(*(p)))

typedef struct fd_set {
    unsigned char fd_bits [(FD_SETSIZE+7)/8];
} fd_set;

void fd_set_or(fd_set *dst, fd_set *src, int maxfdp);
void fd_set_dump(fd_set *set, int maxfdp);

int sys_select(int maxfdp, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
    struct timeval *timeout);
int netif_select(int maxfdp, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
    struct timeval *timeout);

#endif   /* _SYS_SELECT_H */