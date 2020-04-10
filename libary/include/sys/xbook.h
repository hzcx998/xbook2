#ifndef _SYS_XBOOK_H
#define _SYS_XBOOK_H

/* xbook kernel base lib, based on usrmsg */

#include "usrmsg.h"
#include "xfile.h"
#include "types.h"
#include "trigger.h"

/* the name begin with x_ mean xbook basic func for other lib */
typedef unsigned int x_dev_t;  

/* process operations */
int x_fork();
void x_exit(int status);
int x_wait(int *status);
int x_execraw(char *name, char *argv[]);
pid_t x_getpid();
pid_t x_getppid();
int x_execfile(char *name, x_file_t *file, char *argv[]);

x_dev_t x_open(char *name, unsigned long flags);
int x_close(x_dev_t devno);
int x_read(x_dev_t devno, off_t off, void *buf, size_t size);
int x_write(x_dev_t devno, off_t off, void *buf, size_t size);
int x_ioctl(x_dev_t devno, unsigned int cmd, unsigned long arg);
int x_getc(x_dev_t devno, unsigned long *data);
int x_putc(x_dev_t devno, unsigned long data);

/* memory operations */
unsigned long x_heap(unsigned long heap);

/* IPC */
int x_shmget(char *name, size_t size, unsigned long flags);
int x_shmput(int shmid);
void *x_shmmap(int shmid, const void *shmaddr);
int x_shmunmap(const void *shmaddr);
int x_msgget(char *name, int flags);
int x_msgput(int msgid);
int x_msgsnd(int msgid, const void *msgbuf, size_t msgsz, int msgflg);
int x_msgrcv(int msgid, const void *msgbuf, size_t msgsz, long msgtype, int msgflg);
int x_semget(char *name, int value, int flags);
int x_semput(int semid);
int x_semdown(int semid, int semflg);
int x_semup(int semid);
  
int x_trigger(int trig, trighandler_t handler);
int x_trigger_action(int trig, trig_action_t *act, trig_action_t *oldact);
int x_triggeron(int trig, pid_t pid);


#endif  /* _SYS_XBOOK_H */
