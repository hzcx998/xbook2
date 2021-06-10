#include <arch/task.h>
#include <arch/interrupt.h>
#include <arch/segment.h>
#include <xbook/syscall.h>
#include <xbook/memspace.h>
#include <xbook/process.h>
#include <xbook/debug.h>
#include <xbook/schedule.h>
#include <string.h>

void user_frame_init(trap_frame_t *frame)
{
    frame->ds = frame->es = \
    frame->fs = frame->gs = USER_DATA_SEL; 
    frame->cs = USER_CODE_SEL;
    frame->ss = USER_STACK_SEL;

    frame->edi = frame->esi = \
    frame->ebp = frame->esp_dummy = 0;
    frame->eax = frame->ebx = \
    frame->ecx = frame->edx = 0;

    frame->eflags = (EFLAGS_MBS | EFLAGS_IF_1 | EFLAGS_IOPL_0);
}

void kernel_frame_init(trap_frame_t *frame)
{
    frame->ds = frame->es = \
    frame->fs = frame->gs = KERNEL_DATA_SEL; 
    frame->cs = KERNEL_CODE_SEL;
    frame->ss = KERNEL_STACK_SEL;

    frame->edi = frame->esi = \
    frame->ebp = frame->esp_dummy = 0;

    frame->eax = frame->ebx = \
    frame->ecx = frame->edx = 0;

    frame->eflags = (EFLAGS_MBS | EFLAGS_IF_1 | EFLAGS_IOPL_1);
}

void user_thread_frame_build(trap_frame_t *frame, void *arg, void *func,
    void *thread_entry, void *stack_top)
{
    frame->ebx = (unsigned long) arg;
    frame->ecx = (unsigned long) func;
    frame->eip = (unsigned long) thread_entry;
    frame->esp = (unsigned long) stack_top;
}

static void kernel_thread_entry(task_func_t *function, void *arg)
{
    interrupt_enable();  /* 在启动前需要打开中断，避免启动后不能产生时钟中断调度 */
    function(arg);
    // 如何函数返回了，那么就需要调用线程退出
    task_exit(0);
}

void task_stack_build(task_t *task, task_func_t *function, void *arg)
{
    task->kstack -= sizeof(trap_frame_t);
    task->kstack -= sizeof(thread_stack_t);
    thread_stack_t *thread_stack = (thread_stack_t *)task->kstack;

    thread_stack->eip = kernel_thread_entry;
    thread_stack->function = function;
    thread_stack->arg = arg;
    thread_stack->ebp = thread_stack->ebx = \
    thread_stack->esi = thread_stack->edi = 0;
}

int task_stack_build_when_forking(task_t *child)
{

    trap_frame_t *frame = (trap_frame_t *)(
            (unsigned long)child + TASK_KERN_STACK_SIZE - sizeof(trap_frame_t));
    
    /* 设置eax为0，就相当于设置了子任务的返回值为0 */
    frame->eax = 0;

    thread_stack_t *thread_stack = (thread_stack_t *)\
        ((unsigned long *)frame - 5);

    thread_stack->eip = (void *)interrupt_exit;
    
    thread_stack->ebp = thread_stack->ebx = \
    thread_stack->esi = thread_stack->edi = 0;
    
    /* 把构建的线程栈的栈顶最为switch_to恢复数据时的栈顶 */
    child->kstack = (uint8_t *)&thread_stack->ebp;
    return 0;
}

int process_frame_init(task_t *task, trap_frame_t *frame, char **argv, char **envp)
{
    vmm_t *vmm = task->vmm;

    vmm->stack_end = USER_STACK_TOP;
    vmm->stack_start = vmm->stack_end - MEM_SPACE_STACK_SIZE_DEFAULT;

    if (mem_space_mmap(vmm->stack_start, 0, vmm->stack_end - vmm->stack_start , PROT_USER | PROT_WRITE,
        MEM_SPACE_MAP_FIXED | MEM_SPACE_MAP_STACK) == ((void *)-1)) {
        return -1;
    }
    memset((void *) vmm->stack_start, 0, vmm->stack_end - vmm->stack_start);
    
    int argc = 0;
    char **new_envp = NULL;
    unsigned long arg_bottom = 0;
    argc = proc_build_arg(vmm->stack_end, &arg_bottom, envp, &new_envp);
    
    char **new_argv = NULL;
    argc = proc_build_arg(arg_bottom, &arg_bottom, argv, &new_argv);

    frame->ecx = argc;
    frame->ebx = (unsigned int) new_argv;
    frame->edx = (unsigned int) new_envp;

    if (!arg_bottom) {
        mem_space_unmmap(vmm->stack_start, vmm->stack_end - vmm->stack_start);
        return -1;
    }
    frame->esp = (unsigned long) arg_bottom;
    frame->ebp = frame->esp;
    return 0;
}

void thread_kstack_dump(thread_stack_t *kstack)
{
    keprint(PRINT_INFO "eip:%x func:%x arg:%x ebp:%x ebx:%x esi:%x edi:%x\n", 
    kstack->eip, kstack->function, kstack->arg, kstack->ebp, kstack->ebx, kstack->esi, kstack->edi);
}
