#ifndef _XBOOK_PROCESS_H
#define _XBOOK_PROCESS_H

#include "task.h"
#include "elf32.h"
#include <sys/kfile.h>

task_t *user_process_start(char *name, char **argv);
int proc_destroy(task_t *task, int thread);
int proc_vmm_init(task_t *task);
int proc_vmm_exit(task_t *task);
int proc_build_arg(unsigned long arg_top, unsigned long *arg_bottom, char *argv[], char **dest_argv[]);
void proc_map_space_init(task_t *task);
int proc_load_image(vmm_t *vmm, struct Elf32_Ehdr *elf_header, int fd);
void proc_trap_frame_init(task_t *task);
int proc_release(task_t *task);
int proc_trigger_init(task_t *task);
int proc_pthread_init(task_t *task);

int proc_res_init(task_t *task);
int proc_res_exit(task_t *task);

int thread_release_resource(task_t *task);

int sys_fork();
void sys_exit(int status);
pid_t sys_waitpid(pid_t pid, int *status, int options);

int sys_execve(const char *pathname, const char *argv[], const char *envp[]);

#endif /* _XBOOK_PROCESS_H */
