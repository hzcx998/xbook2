#ifndef _XBOOK_PROCESS_H
#define _XBOOK_PROCESS_H

#include "task.h"
#include "elf32.h"
#include "rawblock.h"
#include <sys/kfile.h>
#include <sys/uthread.h>

task_t *process_create(char *name, char **argv);
int proc_destroy(task_t *task, int thread);
int proc_vmm_init(task_t *task);
int proc_vmm_exit(task_t *task);
int proc_build_arg(unsigned long arg_top, char *argv[], char **dest_argv[]);
int proc_stack_init(task_t *task, trap_frame_t *frame, char **argv);
void proc_heap_init(task_t *task);
void proc_map_space_init(task_t *task);
int proc_load_image(vmm_t *vmm, struct Elf32_Ehdr *elf_header, raw_block_t *rb);
void proc_make_trap_frame(task_t *task);
int proc_release(task_t *task);
int proc_trigger_init(task_t *task);
int proc_uthread_init(task_t *task);

int thread_release_resource(task_t *task);

int sys_fork();
void sys_exit(int status);
pid_t sys_waitpid(pid_t pid, int *status, int options);
int sys_exec_raw(char *name, char **argv);
int sys_exec_file(char *name, kfile_t *file, char **argv);

uthread_t sys_thread_create(
    uthread_attr_t *attr,
    task_func_t *func,
    void *arg,
    void *thread_entry
);
void sys_thread_exit(void *retval);
int sys_thread_detach(uthread_t thread);
int sys_thread_join(uthread_t thread, void **thread_return);

#endif /* _XBOOK_PROCESS_H */
