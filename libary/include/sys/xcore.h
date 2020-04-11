#ifndef _SYS_XCORE_H
#define _SYS_XCORE_H

#include "xfile.h"
#include "types.h"
#include "trigger.h"
#include "res.h"

/* process */
pid_t fork();
void exit(int status);
int wait(int *status);
int execraw(char *name, char *argv[]);
int execfile(char *name, xfile_t *file, char *argv[]);
pid_t getpid();
pid_t getppid();

/* vmm */
unsigned long heap(unsigned long heap);

/* device resource */
int writeres(int res, off_t off, void *buffer, size_t size);
int readres(int res, off_t off, void *buffer, size_t size);
int getres(char *name, unsigned long flags, unsigned long arg);
int putres(int res);
int ctlres(int res, unsigned int cmd, unsigned long arg);

int trigger(int trig, trighandler_t handler);
int trigger_action(int trig, trig_action_t *act, trig_action_t *oldact);
int triggeron(int trig, pid_t pid);

#endif  /* _SYS_XCORE_H */
