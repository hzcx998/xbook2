#include <arch/interrupt.h>
#include <arch/task.h>
#include <arch/vmm.h>
#include <xbook/task.h>
#include <xbook/schedule.h>
#include <xbook/process.h>
#include <xbook/memspace.h>
#include <xbook/sharemem.h>
#include <xbook/fd.h>
#include <string.h>

// #define DEBUG_FORK

static pid_t task_fork_pid()
{
    return task_take_pid();
}

/**
 * 在用户态多线程中，fork只会把调用者线程复制给子进程，而其它线程不复制
 */
static int copy_struct_and_kstack(task_t *child, task_t *parent)
{
    memcpy(child, parent, TASK_KERN_STACK_SIZE);
    #ifndef TASK_TRAPFRAME_ON_KSTACK
    child->trapframe = mem_alloc(PAGE_SIZE);
    if (child->trapframe == NULL) {
        errprintln("[fork] copy_struct_and_kstack: malloc trapframe failed!");
        return -1;
    }
    *(child->trapframe) = *(parent->trapframe); /* copy trapframe */
    #endif

    child->pid = task_fork_pid();
    child->tgid = child->pid;
    child->state = TASK_READY;
    child->parent_pid = parent->pid;
    child->pgid = parent->pgid;     /* 和父进程在同一个组 */
    list_init(&child->list);
    list_init(&child->global_list);
    #ifndef TASK_TRAPFRAME_ON_KSTACK
    child->kstack = (unsigned char *)((unsigned char *)child + TASK_KERN_STACK_SIZE - sizeof(trap_frame_t));
    #else
    child->kstack = (unsigned char *)((unsigned char *)child + TASK_KERN_STACK_SIZE);
    #endif
    #ifndef TASK_TINY 
    child->port_comm = NULL;
    #endif
    return 0;
}

static void copy_vm_struct(task_t *child, task_t *parent)
{
    child->vmm->code_start = parent->vmm->code_start;
    child->vmm->data_start = parent->vmm->data_start;
    child->vmm->heap_start = parent->vmm->heap_start;
    child->vmm->map_start = parent->vmm->map_start;
    child->vmm->stack_start = parent->vmm->stack_start;
    child->vmm->code_end = parent->vmm->code_end;
    child->vmm->data_end = parent->vmm->data_end;
    child->vmm->heap_end = parent->vmm->heap_end;
    child->vmm->map_end = parent->vmm->map_end;
    child->vmm->stack_end = parent->vmm->stack_end;
}

static int copy_vm(task_t *child, task_t *parent)
{
    if (proc_vmm_init(child) < 0)
        return -1;
    copy_vm_struct(child, parent);
    if (vmm_copy_mem_space(child->vmm, parent->vmm) < 0) {
        vmm_free(child->vmm);
        child->vmm = NULL;
        return -1;
    }
    if (vmm_copy_mapping(child, parent) < 0) {
        vmm_release_space(child->vmm);
        vmm_free(child->vmm);
        child->vmm = NULL;
        return -1;
    }
    return 0;
}

static int copy_file(task_t *child, task_t *parent)
{
    if (fs_fd_init(child) < 0)
        return -1;
    return fs_fd_copy(parent, child);
}

static int copy_pthread_desc(task_t *child, task_t *parent)
{
    #ifndef TASK_TINY 
    if (parent->pthread != NULL) { 
        /* 由于复制后，只有一个线程，所以这里就直接初始化一个新的，而不是复制 */
        if (proc_pthread_init(child))
            return -1;
    }
    #endif
    return 0;
}

static int copy_exception(task_t *child, task_t *parent)
{
    exception_manager_init(&child->exception_manager);
    return exception_copy(&child->exception_manager, &parent->exception_manager);
}

static int copy_task(task_t *child, task_t *parent)
{
    if (copy_struct_and_kstack(child, parent))
        goto rollback_failed;
    if (copy_vm(child, parent))
        goto rollback_struct;
    if (copy_pthread_desc(child, parent))
        goto rollback_vmm;
    if (copy_file(child, parent) < 0)
        goto rollback_pthread_desc;
    if (copy_exception(child, parent) < 0)
        goto rollback_file;
    task_stack_build_when_forking(child);
    return 0;
rollback_file:
    fs_fd_exit(child);
rollback_pthread_desc:
    proc_pthread_exit(child);
rollback_vmm:
    proc_vmm_exit_when_forking(child, parent);
rollback_struct:
    task_rollback_pid();
rollback_failed:
    return -1;    
}

int sys_fork()
{
    task_t *parent = task_current;
    dbgprintln("[fork] task %s pid=%d forking...", parent->name, parent->pid);
    unsigned long flags;
    interrupt_save_and_disable(flags);
    task_t *child = mem_alloc(TASK_KERN_STACK_SIZE);
    if (child == NULL) {
        keprint(PRINT_ERR "sys_fork: mem_alloc for child task failed!\n");
        return -1;
    }
    assert(parent->vmm != NULL);
    if (copy_task(child, parent)) {
        keprint(PRINT_ERR "sys_fork: copy task failed!\n");
        mem_free(child);
        interrupt_restore_state(flags);
        return -1;
    }
    task_add_to_global_list(child);
    sched_queue_add_tail(sched_get_cur_unit(), child);
    interrupt_restore_state(flags);
    // trap_frame_dump(parent->trapframe);
    dbgprintln("[fork] parent %s pid=%d forked child %s pid=%d", parent->name, parent->pid, child->name, child->pid);
    
    return child->pid;  /* 父进程返回子进程pid */
}