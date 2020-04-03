#ifndef _SYS_IPC_H
#define _SYS_IPC_H

/* IPC flags */
#define IPC_CREAT   0x01
#define IPC_EXCL    0x02
#define IPC_NOWAIT  0x04
#define IPC_NOERROR 0x08
#define IPC_EXCEPT  0x10

/* message buf */
typedef struct {
    long type;      /* msg type */
    char text[1];   /* msg text */
} x_msgbuf_t;

#endif   /* _SYS_IPC_H */