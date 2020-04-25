#ifndef _SYS_PROC_H
#define _SYS_PROC_H

#include "kfile.h"
#include "types.h"

/* process */
pid_t fork();
void exit(int status);
int wait(int *status);
int waitpid(pid_t pid, int *status, int options);
int execraw(char *name, char *argv[]);
int execfile(char *name, kfile_t *file, char *argv[]);
pid_t getpid();
pid_t getppid();
pid_t gettid();
unsigned long sleep(unsigned long second);

#endif  /* _SYS_PROC_H */
