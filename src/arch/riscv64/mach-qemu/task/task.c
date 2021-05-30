#include <xbook/task.h>
#include <xbook/schedule.h>
#include <xbook/process.h>
#include <xbook/memspace.h>
#include <arch/phymem.h>
#include <arch/interrupt.h>
#include <arch/riscv.h>
#include <arch/page.h>

extern char trampoline[], uservec[], userret[];

static void kernel_thread_entry()
{
    task_t *cur = task_current;
    interrupt_enable();  /* 在启动前需要打开中断，避免启动后不能产生时钟中断调度 */
    cur->kthread_entry(cur->kthread_arg);
    // 如果函数返回了，那么就需要调用线程退出
    kern_thread_exit(0);
}

void task_stack_build(task_t *task, task_func_t *function, void *arg)
{
    // Set up new context to start executing at forkret,
    // which returns to user space.
    memset(&task->context, 0, sizeof(task->context));
    task->context.ra = (uint64_t)kernel_thread_entry;      // 函数执行入口
    task->context.sp = (uint64_t)(task->kstack + PAGE_SIZE); 
    task->kthread_entry = function;
    task->kthread_arg = arg;
}

/**
 * 将参数构建到栈中
 */
static int build_arg_stack(vmm_t *vmm, unsigned long stackbase, unsigned long *_sp, char **argv)
{
    unsigned long sp = *_sp;
    uint64_t argc;
    uint64_t ustack[MAX_TASK_STACK_ARG_NR + 1];
    // Push argument strings, prepare rest of stack in ustack.
    for(argc = 0; argv[argc]; argc++) {
        if(argc >= MAX_TASK_STACK_ARG_NR)
            return -1;
        sp -= strlen(argv[argc]) + 1;
        sp -= sp % 16; // riscv sp must be 16-byte aligned
        if(sp < stackbase)
            return -1;
        if(page_copy_out(vmm->page_storage, sp, argv[argc], strlen(argv[argc]) + 1) < 0)
            return -1;
        ustack[argc] = sp;
    }
    ustack[argc] = 0;
    // push the array of argv[] pointers.
    sp -= (argc+1) * sizeof(uint64_t);
    sp -= sp % 16;
    if(sp < stackbase)
        return -1;
    if(page_copy_out(vmm->page_storage, sp, (char *)ustack, (argc+1)*sizeof(uint64_t)) < 0)
        return -1;
    
    // save argc
    sp -= sizeof(uint64_t);
    if(page_copy_out(vmm->page_storage, sp, (char *)&argc, sizeof(uint64_t)) < 0)
        return -1;
    // save sp as new value
    *_sp = sp;
    return 0;
} 


int process_frame_init(task_t *task, vmm_t *vmm, trap_frame_t *frame, char **argv, char **envp)
{
    vmm->stack_end = USER_STACK_TOP;
    vmm->stack_start = vmm->stack_end - MEM_SPACE_STACK_SIZE_DEFAULT;

    if (mem_space_mmap2(vmm, vmm->stack_start, 0, vmm->stack_end - vmm->stack_start , PROT_USER | PROT_WRITE | PROT_READ,
        MEM_SPACE_MAP_FIXED | MEM_SPACE_MAP_STACK) == ((void *)-1)) {
        return -1;
    }
    /**
     * 参数布局：
     * --------*
     * env arg *
     * --------*
     * arg     *
     * --------*
     * argc    * <- sp
     * --------*
     */
    unsigned long arg_bottom = 0;
    arg_bottom = vmm->stack_end;
    if (build_arg_stack(vmm, vmm->stack_end - PAGE_SIZE, &arg_bottom, argv) < 0) {
        mem_space_unmmap2(vmm, vmm->stack_start, vmm->stack_end - vmm->stack_start);
        return -1;
    }
    /* sp保存参数信息，通过sp就可以找到所有参数了 */
    frame->sp = arg_bottom;
    return 0;
}

void kernel_switch_to_user(trap_frame_t *frame)
{
    task_activate_when_sched(task_current);  // 激活页表
    usertrapret();
}

void user_frame_init(trap_frame_t *frame)
{
    memset(frame, 0, sizeof(trap_frame_t));
}

extern pgdir_t kernel_pgdir;
int task_stack_build_when_forking(task_t *child)
{
    trap_frame_t *frame = child->trapframe;
    /* 设置a0为0，就相当于设置了子任务的返回值为0 */
    frame->a0 = 0;
    memset(&child->context, 0, sizeof(child->context));
    child->context.ra = (uint64_t)forkret;      // 子进程第一次获得执行权时会跳转到该处
    child->context.sp = (uint64_t)(child->kstack + PAGE_SIZE); 
    return 0;
}

void proc_set_stack_pointer(task_t *task, unsigned long sp)
{
    task->trapframe->sp = sp;
}