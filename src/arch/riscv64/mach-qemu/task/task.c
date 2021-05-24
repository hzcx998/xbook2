#include <xbook/task.h>
#include <xbook/schedule.h>
#include <xbook/process.h>
#include <xbook/memspace.h>
#include <k210_qemu_phymem.h>
#include <arch/interrupt.h>
#include <arch/riscv.h>
#include <arch/page.h>

extern char trampoline[], uservec[], userret[];

static void kernel_thread_entry()
{
    task_t *cur = task_current;
    interrupt_enable();  /* 在启动前需要打开中断，避免启动后不能产生时钟中断调度 */
    cur->kthread_entry(cur->kthread_arg);
    // 如何函数返回了，那么就需要调用线程退出
    #if 1
    panic("kthread return!");
    #else
    kern_thread_exit(0);
    #endif
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
    keprint("task=%s sp=%p\n", task->name, task->context.sp);
}

int process_frame_init(task_t *task, vmm_t *vmm, trap_frame_t *frame, char **argv, char **envp)
{
    vmm->stack_end = USER_STACK_TOP;
    vmm->stack_start = vmm->stack_end - MEM_SPACE_STACK_SIZE_DEFAULT;

    if (mem_space_mmap2(vmm, vmm->stack_start, 0, vmm->stack_end - vmm->stack_start , PROT_USER | PROT_WRITE | PROT_READ,
        MEM_SPACE_MAP_FIXED | MEM_SPACE_MAP_STACK) == ((void *)-1)) {
        return -1;
    }
    
    int argc = 0;
    char **new_envp = NULL;
    unsigned long arg_bottom = 0;
    char **new_argv = NULL;
    #if 0
    argc = proc_build_arg(vmm->stack_end, &arg_bottom, envp, &new_envp);
    argc = proc_build_arg(arg_bottom, &arg_bottom, argv, &new_argv);
    
    if (!arg_bottom) {
        mem_space_unmmap(vmm->stack_start, vmm->stack_end - vmm->stack_start);
        return -1;
    }
    #else
    arg_bottom = vmm->stack_end;
    #endif
    /* 传参数 */
    #if 0
    frame->ecx = argc;
    frame->ebx = (unsigned int) new_argv;
    frame->edx = (unsigned int) new_envp;

    frame->esp = (unsigned long) arg_bottom;
    frame->ebp = frame->esp;
    #else
    frame->a0 = argc;
    frame->a1 = (uint64_t) new_argv;
    frame->a2 = (uint64_t) new_envp;
    frame->sp = (uint64_t) arg_bottom;
    #endif
    return 0;
}

void kernel_switch_to_user(trap_frame_t *frame)
{
    noteprintln("kernel_switch_to_user: is empty function");
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