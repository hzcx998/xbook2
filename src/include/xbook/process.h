#ifndef _XBOOK_PROCESS_H
#define _XBOOK_PROCESS_H

#include "task.h"
#include <sys/usrmsg.h>

task_t *process_create(char *name, char **argv);
int proc_vmm_init(task_t *task);
int do_usrmsg_fork(umsg_t *msg);


#endif /* _XBOOK_PROCESS_H */
