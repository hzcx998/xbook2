#ifndef _XBOOK_PROCESS_H
#define _XBOOK_PROCESS_H

#include "task.h"
#include <sys/usrmsg.h>

task_t *process_create(char *name, char **argv);
int proc_vmm_init(task_t *task);
int proc_vmm_exit(task_t *task);

int proc_fork(long *retval);
void proc_exit(int status);
pid_t proc_wait(int *status);

#endif /* _XBOOK_PROCESS_H */
