#include <arch/interrupt.h>
#include <arch/task.h>
#include <arch/vmm.h>
#include <xbook/task.h>
#include <xbook/schedule.h>
#include <xbook/process.h>
#include <xbook/memspace.h>
#include <xbook/sharemem.h>
#include <fsal/fsal.h>
#include <gui/message.h>

#include <string.h>

// #define DEBUG_FORK

/**
 * 在用户态多线程中，fork只会把调用者线程复制给子进程，而其它线程不复制
 */
static int copy_struct_and_kstack(task_t *child, task_t *parent)
{
    memcpy(child, parent, TASK_KERN_STACK_SIZE);
    child->pid = task_fork_pid();
    child->tgid = child->pid;
    child->state = TASK_READY;
    child->parent_pid = parent->pid;
    INIT_LIST_HEAD(&child->list);
    INIT_LIST_HEAD(&child->global_list);
    child->kstack = (unsigned char *)((unsigned char *)child + TASK_KERN_STACK_SIZE - sizeof(trap_frame_t));
    return 0;
}

static int copy_vm_struct(task_t *child, task_t *parent)
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
    return 0;
}

/**
 * 复制共享内存，共享内存有可能是以其他形式存在的，并不是shmid这种。
 * 只有增长引用计数失败才返回
 */
static int copy_share_mem(mem_space_t *mem_space)
{
    addr_t phyaddr = addr_vir2phy(mem_space->start);  
    share_mem_t *shm = share_mem_find_by_addr(phyaddr);
    if (shm == NULL) { 
        return 0;
    }
    return share_mem_grow(shm->id);
}

static int copy_vm_mem_space(task_t *child, task_t *parent)
{
    mem_space_t *tail = NULL;
    mem_space_t *p = parent->vmm->mem_space_head;
    while (p != NULL) {
        mem_space_t *space = mem_space_alloc();
        if (space == NULL) {
            printk(KERN_ERR "copy_vm_mem_space: mem_alloc for space failed!\n");
            return -1;
        }
        *space = *p;
        space->next = NULL;
        if (space->flags & MEM_SPACE_MAP_SHARED) {
            if (copy_share_mem(space) < 0)
                return -1;
        }
        if (tail == NULL)
            child->vmm->mem_space_head = space;    
        else 
            tail->next = space;
        tail = space;
        p = p->next;
    }
    return 0;
}

static int copy_vm(task_t *child, task_t *parent)
{
    if (copy_vm_struct(child, parent))
        return -1;
    if (vmm_copy_mapping(child, parent))
        return -1;
    if (copy_vm_mem_space(child, parent))
        return -1;
    return 0;
}

static int copy_trigger(task_t *child, task_t *parent)
{
    child->triggers = mem_alloc(sizeof(triggers_t));
    if (child->triggers == NULL)
        return -1;
    *child->triggers = *parent->triggers;
    return 0;
}

static int copy_file(task_t *child, task_t *parent)
{
    if (fs_fd_init(child) < 0)
        return -1;
    return fs_fd_copy(parent, child);
}

static int copy_gui(task_t *child, task_t *parent)
{
    // 不复制图形消息池
    if (parent->gmsgpool) {
        child->gmsgpool = NULL;
    }
    return 0;
}

static int copy_pthread_desc(task_t *child, task_t *parent)
{
    if (parent->pthread != NULL) { 
        /* 由于复制后，只有一个线程，所以这里就直接初始化一个新的，而不是复制 */
        if (proc_pthread_init(child))
            return -1;
    }
    return 0;
}

static int copy_task(task_t *child, task_t *parent)
{
    // TODO: 回滚复制失败后的资源
    if (copy_struct_and_kstack(child, parent))
        return -1;
    if (proc_vmm_init(child)) {
        return -1;
    }
    if (copy_vm(child, parent))
        return -1;
    if (copy_trigger(child, parent))
        return -1;
    if (copy_pthread_desc(child, parent))
        return -1;
    if (copy_file(child, parent) < 0)
        return -1; 
    if (copy_gui(child, parent) < 0)
        return -1;
    task_stack_build_when_forking(child);
    return 0;
}

int sys_fork()
{
    task_t *parent = task_current;
    unsigned long flags;
    interrupt_save_and_disable(flags);
    task_t *child = mem_alloc(TASK_KERN_STACK_SIZE);
    if (child == NULL) {
        printk(KERN_ERR "do_usrmsg_fork: mem_alloc for child task failed!\n");
        return -1;
    }
    ASSERT(parent->vmm != NULL);
    if (copy_task(child, parent)) {
        printk(KERN_ERR "do_usrmsg_fork: copy task failed!\n");
        mem_free(child);
        return -1;
    }
    task_add_to_global_list(child);
    sched_queue_add_tail(sched_get_cur_unit(), child);
    interrupt_restore_state(flags);
    return child->pid;  /* 父进程返回子进程pid */
}