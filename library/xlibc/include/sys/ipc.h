#ifndef _SYS_IPC_H
#define _SYS_IPC_H

#include <types.h>

/* IPC local flags */
#define IPC_CREAT   0x01        /* create a ipc */
#define IPC_EXCL    0x02        /* must open a not exist ipc */
#define IPC_NOWAIT  0x04        /* no wait */
#define IPC_NOERROR 0x08        /* no error */
#define IPC_EXCEPT  0x10        /* except something */
#define IPC_READER  0x20        /* reader for pipe */
#define IPC_WRITER  0x40        /* writer for pipe */
#define IPC_NOSYNC  0x80        /* no sync */
#define IPC_RND     0x100       /* addr round page align */
#define IPC_REMAP   0x200       /* remap memory */

/* IPC slaver flags */
#define IPC_SHM     0x100000    /* share memory ipc */
#define IPC_SEM     0x200000    /* semaphore ipc */
#define IPC_MSG     0x400000    /* message queue ipc */
#define IPC_PIPE    0x800000    /* pipe ipc */

/* IPC cmd */
#define IPC_DEL     1           /* del a ipc from kernel */
#define IPC_SETRW   2           /* set ipc reader writer */
#define IPC_SET     3           /* set ipc */

#define IPC_NAME_LEN   24       /* ipc name len */

/* message buf */
typedef struct {
    long type;      /* msg type */
    char text[1];   /* msg text */
} kmsgbuf_t;

int shmget(char *name, unsigned long size, unsigned long flags);
int shmput(int shmid);
void *shmmap(int shmid, void *shmaddr, int shmflg);
int shmunmap(const void *shmaddr, int shmflg);
int semget(char *name, int value, int semflg);
int semput(int semid);
int semdown(int semid, int semflg);
int semup(int semid);
int msgget(char *name, unsigned long flags);
int msgput(int msgid);
int msgsend(int msgid, void *msgbuf, size_t size, int msgflg);
int msgrecv(int msgid, void *msgbuf, size_t msgsz, int msgflg);

#endif   /* _SYS_IPC_H */