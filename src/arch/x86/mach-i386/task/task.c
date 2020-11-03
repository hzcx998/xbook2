#include <arch/task.h>
#include <arch/interrupt.h>
#include <arch/segment.h>
#include <xbook/syscall.h>
#include <xbook/trigger.h>
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

void trigger_frame_build(trap_frame_t *frame, int trig, void *_act)
{
    trig_action_t *act = (trig_action_t *)_act;
    trigger_frame_t *trigger_frame = (trigger_frame_t *)((frame->esp - sizeof(trigger_frame_t)) & -8UL);
    
    trigger_frame->trig = trig;
    trigger_frame->oldmask = current_task->triggers->blocked;

    memcpy(&trigger_frame->trap_frame, frame, sizeof(trap_frame_t));

    trigger_frame->ret_addr = trigger_frame->ret_code;

    /* 构建返回代码，系统调用封装
    模拟系统调用来实现从用户态返回到内核态
    mov eax, SYS_TRIGRET
    int 0x40
     */
    trigger_frame->ret_code[0] = 0xb8;                          /* 给eax赋值的机器码 */
    *(uint32_t *)(trigger_frame->ret_code + 1) = SYS_TRIGRET;   /* 系统调用号 */
    *(uint16_t *)(trigger_frame->ret_code + 5) = 0x40cd;        /* int对应的指令是0xcd，系统调用中断号是0x40 */
    
    frame->eip = (unsigned int)act->handler;
    frame->esp = (unsigned int)trigger_frame;
    frame->ds = frame->es = frame->fs = frame->gs = USER_DATA_SEL;
    frame->ss = USER_STACK_SEL;
    frame->cs = USER_CODE_SEL;
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
            (unsigned long)child + TASK_KSTACK_SIZE - sizeof(trap_frame_t));
    
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

int trigger_return_to_user(trap_frame_t *frame)
{
    /* 原本trigger_frame是在用户栈esp-trigger_frame_size这个位置，但是由于调用了用户处理程序后，
    函数会把返回地址弹出，也就是esp+4，所以，需要通过esp-4才能获取到trigger_frame */
    trigger_frame_t *trigger_frame = (trigger_frame_t *)(frame->esp - 4);
    trigset_t oldset = trigger_frame->oldmask;
    
    triggers_t *trigger = current_task->triggers; 
    spin_lock_irq(&trigger->trig_lock);
    trigger->blocked = oldset;
    trigger_calc_left(trigger);
    spin_unlock_irq(&trigger->trig_lock);
    memcpy(frame, &trigger_frame->trap_frame, sizeof(trap_frame_t));
    return frame->eax;
}

void thread_kstack_dump(thread_stack_t *kstack)
{
    printk(KERN_INFO "eip:%x func:%x arg:%x ebp:%x ebx:%x esi:%x edi:%x\n", 
    kstack->eip, kstack->function, kstack->arg, kstack->ebp, kstack->ebx, kstack->esi, kstack->edi);
}
