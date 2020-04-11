#ifndef _SYS_IPC_H
#define _SYS_IPC_H

/* IPC flags */
#define IPC_CREAT   0x01
#define IPC_EXCL    0x02
#define IPC_NOWAIT  0x04
#define IPC_NOERROR 0x08
#define IPC_EXCEPT  0x10

#define IPC_SHM     0x100000
#define IPC_SEM     0x200000
#define IPC_MSG     0x400000

/* IPC cmd */
#define SHM_MAP     1
#define SHM_UNMAP   2

#define SEM_DOWN    1
#define SHM_UP      2

#define MSG_SND     1
#define MSG_RECV    2

/* message buf */
typedef struct {
    long type;      /* msg type */
    char text[1];   /* msg text */
} x_msgbuf_t;

#endif   /* _SYS_IPC_H */