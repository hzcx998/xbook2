#ifndef _SYS_XBOOK_H
#define _SYS_XBOOK_H

/* xbook kernel base lib, based on usrmsg */

#include "usrmsg.h"
#include "xfile.h"

/* the name begin with x_ mean xbook basic func for other lib */
typedef unsigned int x_dev_t;  
typedef unsigned long x_off_t;  
typedef unsigned long x_count_t;  
typedef unsigned long x_size_t;

/* process operations */
int x_fork();
void x_exit(int status);
int x_wait(int *status);
int x_execraw(char *name, char *argv[]);
int x_execfile(char *name, x_file_t *file, char *argv[]);
/* device operations */

x_dev_t x_open(char *name, unsigned long flags);
int x_close(x_dev_t devno);
int x_read(x_dev_t devno, x_off_t off, void *buf, x_size_t size);
int x_write(x_dev_t devno, x_off_t off, void *buf, x_size_t size);
int x_ioctl(x_dev_t devno, unsigned int cmd, unsigned long arg);
int x_getc(x_dev_t devno, unsigned long *data);
int x_putc(x_dev_t devno, unsigned long data);

/* memory operations */
unsigned long x_heap(unsigned long heap);

/* IPC */
int x_shmget(char *name, x_size_t size, unsigned long flags);
int x_shmput(int shmid);
void *x_shmmap(int shmid, const void *shmaddr);
int x_shmunmap(const void *shmaddr);
int x_msgget(char *name, int flags);
int x_msgput(int msgid);
int x_msgsnd(int msgid, const void *msgbuf, x_size_t msgsz, int msgflg);
int x_msgrcv(int msgid, const void *msgbuf, x_size_t msgsz, long msgtype, int msgflg);

#endif  /* _SYS_XBOOK_H */
