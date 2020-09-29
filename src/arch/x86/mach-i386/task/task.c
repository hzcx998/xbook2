#include <arch/task.h>
#include <arch/interrupt.h>
#include <arch/segment.h>
#include <xbook/syscall.h>
#include <xbook/trigger.h>
#include <xbook/vmspace.h>
#include <xbook/process.h>
#include <xbook/debug.h>
#include <string.h>

void __user_trap_frame_init(trap_frame_t *frame)
{
    /* 数据段 */
    frame->ds = frame->es = \
    frame->fs = frame->gs = USER_DATA_SEL; 

    /* 代码段 */
    frame->cs = USER_CODE_SEL;

    /* 栈段 */
    frame->ss = USER_STACK_SEL;

    /* 设置通用寄存器的值 */
    frame->edi = frame->esi = \
    frame->ebp = frame->esp_dummy = 0;

    frame->eax = frame->ebx = \
    frame->ecx = frame->edx = 0;
    
    /* 为了能让程序首次运行，这里需要设置eflags的值 */
    frame->eflags = (EFLAGS_MBS | EFLAGS_IF_1 | EFLAGS_IOPL_0);
}

void __ktask_trap_frame_init(trap_frame_t *frame)
{
    /* 数据段 */
    frame->ds = frame->es = \
    frame->fs = frame->gs = SERVE_DATA_SEL; 

    /* 代码段 */
    frame->cs = SERVE_CODE_SEL;

    /* 栈段 */
    frame->ss = SERVE_STACK_SEL;

    /* 设置通用寄存器的值 */
    frame->edi = frame->esi = \
    frame->ebp = frame->esp_dummy = 0;

    frame->eax = frame->ebx = \
    frame->ecx = frame->edx = 0;

    /* 为了能让程序首次运行，这里需要设置eflags的值 */
    frame->eflags = (EFLAGS_MBS | EFLAGS_IF_1 | EFLAGS_IOPL_1);
}

void __kernel_trap_frame_init(trap_frame_t *frame)
{
    /* 数据段 */
    frame->ds = frame->es = \
    frame->fs = frame->gs = KERNEL_DATA_SEL; 
    
    /* 代码段 */
    frame->cs = KERNEL_CODE_SEL;

    /* 栈段 */
    frame->ss = KERNEL_STACK_SEL;

    /* 设置通用寄存器的值 */
    frame->edi = frame->esi = \
    frame->ebp = frame->esp_dummy = 0;

    frame->eax = frame->ebx = \
    frame->ecx = frame->edx = 0;

    /* 为了能让程序首次运行，这里需要设置eflags的值 */
    frame->eflags = (EFLAGS_MBS | EFLAGS_IF_1 | EFLAGS_IOPL_1);
}


void __build_trigger_frame(int trig, void *_act, trap_frame_t *frame)
{
    trig_action_t *act = (trig_action_t *)_act;

    /* 获取信号栈框，在用户的esp栈下面 */
    trigger_frame_t *trigger_frame = (trigger_frame_t *)((frame->esp - sizeof(trigger_frame_t)) & -8UL);
    /*
    printk("trap frame %x, signal frame esp %x, %x\n", 
    frame->esp, frame->esp - sizeof(trigger_frame_t), (frame->esp - sizeof(trigger_frame_t)) & -8UL);
    */
    /* 传递给handler的参数 */
    trigger_frame->trig = trig;
    
    trigger_frame->oldmask = current_task->triggers->blocked;

    /* 把中断栈保存到信号栈中 */
    memcpy(&trigger_frame->trap_frame, frame, sizeof(trap_frame_t));

    /* 设置返回地址为构建的返回代码 */
    trigger_frame->ret_addr = trigger_frame->ret_code;

    /* 构建返回代码，系统调用封装
    模拟系统调用来实现从用户态返回到内核态
    mov eax, SYS_TRIGRET
    int 0x40
     */
    trigger_frame->ret_code[0] = 0xb8; /* 给eax赋值的机器码 */
    *(uint32_t *)(trigger_frame->ret_code + 1) = SYS_TRIGRET;    /* 把系统调用号填进去 */
    *(uint16_t *)(trigger_frame->ret_code + 5) = 0x40cd;      /* int对应的指令是0xcd，系统调用中断号是0x40 */
    
    /* 设置中断栈的eip成为用户设定的处理函数 */
    frame->eip = (unsigned int)act->handler;

    /* 设置运行时的栈 */
    frame->esp = (unsigned int)trigger_frame;

    /* 设置成用户态的段寄存器 */
    frame->ds = frame->es = frame->fs = frame->gs = USER_DATA_SEL;
    frame->ss = USER_STACK_SEL;
    frame->cs = USER_CODE_SEL;
}

void __build_user_thread_frame(trap_frame_t *frame, void *arg, void *func,
    void *thread_entry, void *stack_top)
{
    frame->ebx = (unsigned long) arg;     /* 参数 */
    frame->ecx = (unsigned long) func;    /* 线程例程入口 */
    frame->eip = (unsigned long) thread_entry;
    frame->esp = (unsigned long) stack_top;
}

/**
 * KernelThread - 执行内核线程
 * @function: 要执行的线程
 * @arg: 参数
 * 
 * 改变当前的执行流，去执行我们选择的内核线程
 */
static void kernel_thread(task_func_t *function, void *arg)
{
    enable_intr();  /* 在启动前需要打开中断，避免启动后不能产生时钟中断调度 */
    function(arg);
}

/**
 * __make_task_stack - 创建一个线程的执行环境
 * @task: 线程结构体
 * @function: 要去执行的函数
 * @arg: 参数
 */
void __make_task_stack(task_t *task, task_func_t *function, void *arg)
{
    /* 预留中断栈 */
    task->kstack -= sizeof(trap_frame_t);
    /* 预留线程栈 */
    task->kstack -= sizeof(thread_stack_t);
    thread_stack_t *thread_stack = (thread_stack_t *)task->kstack;

    /* 填写线程栈信息 */
    // 在kernel_thread中去改变执行流，从而可以传递一个参数
    thread_stack->eip = kernel_thread;
    thread_stack->function = function;
    thread_stack->arg = arg;
    thread_stack->ebp = thread_stack->ebx = \
    thread_stack->esi = thread_stack->edi = 0;

}

int __fork_bulid_child_stack(task_t *child)
{
    /* 1.让子进程返回0 */

    /* 获取中断栈框 */
    trap_frame_t *frame = (trap_frame_t *)(
            (unsigned long)child + TASK_KSTACK_SIZE - sizeof(trap_frame_t));
    /*
	printk("edi: %x esi: %x ebp: %x esp: %x\n", 
			frame->edi, frame->esi, frame->ebp, frame->esp);
	printk("ebx: %x edx: %x ecx: %x eax: %x\n", 
			frame->ebx, frame->edx, frame->ecx, frame->eax);
	printk("gs: %x fs: %x es: %x ds: %x\n", 
			frame->gs, frame->fs, frame->es, frame->ds);
	printk("err: %x eip: %x cs: %x eflags: %x\n", 
			frame->errorCode, frame->eip, frame->cs, frame->eflags);
	printk("esp: %x ss: %x\n", 
			frame->esp, frame->ss);
	*/

    //printk(PART_TIP "task at %x fram at %x\n", child, frame);
    /* 设置eax为0，就相当于设置了子任务的返回值为0 */
    frame->eax = 0;

    /* 线程栈我们需要的数据只有5个，即ebp，ebx，edi，esi，eip */
    thread_stack_t *thread_stack = (thread_stack_t *)\
        ((unsigned long *)frame - 5);

    /* 把SwitchTo的返回地址设置成InterruptExit，直接从中断返回 */
    //thread_stack->eip = (unsigned long)&InterruptExit;
    thread_stack->eip = (void *)intr_exit;
    
    //printk(PART_TIP "thread_stack eip %x\n", thread_stack->eip);
    /* 下面的赋值只是用来使线程栈构建更清晰，下面2行的赋值其实不必要，
    因为在进入InterruptExit之后会进行一系列pop把寄存器覆盖掉 */
    thread_stack->ebp = thread_stack->ebx = \
    thread_stack->esi = thread_stack->edi = 0;
    
    /* 把构建的线程栈的栈顶最为switch_to恢复数据时的栈顶 */
    child->kstack = (uint8_t *)&thread_stack->ebp;
    //printk(PART_TIP "kstack %x\n", child->kstack);
    
    /* 2.为SwitchTo构建线程环境，在中断栈框下面 */
    //unsigned long *retAddrInthread_stack = (unsigned long *)frame - 1;

    /* 这3行只是为了梳理线程栈中的关系，不一定要写出来 */
    /* unsigned long *esiInInthread_stack = (unsigned long *)frame - 2;
    unsigned long *ediInInthread_stack = (unsigned long *)frame - 3;
    unsigned long *ebxInInthread_stack = (unsigned long *)frame - 4; */
    /* ebp在线程栈中的地址便是当时esp（0级栈的栈顶），也就是esp
    为 "(unsigned long *)frame - 5"*/
    //unsigned long *ebpInInthread_stack = (unsigned long *)frame - 5;

    /* 把SwitchTo的返回地址设置成InterruptExit，直接从中断返回 */
    // *retAddrInthread_stack = (unsigned long)&InterruptExit;

    /* 下面的赋值只是用来使线程栈构建更清晰，下面2行的赋值其实不必要，
    因为在进入InterruptExit之后会进行一系列pop把寄存器覆盖掉 */
    /* *ebpInInthread_stack = *ebxInInthread_stack = \
    *ediInInthread_stack = *esiInInthread_stack = 0;*/

    /* 把构建的线程栈的栈顶最为SwitchTo恢复数据时的栈顶 */
    //child->kstack = (uint8_t *)ebpInInthread_stack;

    return 0;
}

/**
 * __proc_stack_init - 初始化用户栈和参数
 * 
 */
int __proc_stack_init(task_t *task, trap_frame_t *frame, char **argv, char **envp)
{
    vmm_t *vmm = task->vmm;

    vmm->stack_end = USER_STACK_TOP;
    vmm->stack_start = vmm->stack_end - DEFAULT_STACK_SIZE; /* 栈大小 */

    /* 固定位置，初始化时需要一个固定位置，向下拓展时才动态。 */
    if (vmspace_mmap(vmm->stack_start, 0, vmm->stack_end - vmm->stack_start , PROT_USER | PROT_WRITE,
        VMS_MAP_FIXED | VMS_MAP_STACK) == ((void *)-1)) {
        return -1;
    }
    memset((void *) vmm->stack_start, 0, vmm->stack_end - vmm->stack_start);
    
    int argc = 0;
    char **new_envp = NULL;
    unsigned long arg_bottom = 0;
    argc = proc_build_arg(vmm->stack_end, &arg_bottom, envp, &new_envp);
    
    char **new_argv = NULL;
    argc = proc_build_arg(arg_bottom, &arg_bottom, argv, &new_argv);

    /* 记录参数个数 */
    frame->ecx = argc;
    /* 记录参数地址 */
    frame->ebx = (unsigned int) new_argv;
    /* 记录环境地址 */
    frame->edx = (unsigned int) new_envp;

     /* 记录栈顶 */
    if (!arg_bottom) {  /* 解析参数出错 */
        vmspace_unmmap(vmm->stack_start, vmm->stack_end - vmm->stack_start);
        return -1;
    }
    frame->esp = (unsigned long) arg_bottom; /* 栈位于参数的下面 */

    frame->ebp = frame->esp;
#if 0 /* stack info */
    printk(KERN_DEBUG "task %s arg space: start %x end %x\n",
        (current_task)->name, vmm->arg_start, vmm->arg_end);
    printk(KERN_DEBUG "task %s stack space: start %x end %x\n",
        (current_task)->name, vmm->stack_start, vmm->stack_end);
    
    printk(KERN_DEBUG "stack %x argc %d argv %x\n", frame->esp, frame->ecx, frame->ebx);
#endif
#if 0 /* print args */
    int i = 0;
    while (new_argv[i]) {
        printk("[%x %x]", new_argv[i], new_argv[i][0]);
        
        printk("%s ", new_argv[i]);
        i++;
    }
#endif
    return 0;
}

/**
 * __trigger_return - 执行完可捕捉触发器后返回
 * @frame: 栈
 * 
 */
int __trigger_return(trap_frame_t *frame)
{
    //printk("arg is %x %x %x %x %x\n", ebx, ecx, esi, edi, frame);

    //printk("usr esp - 4:%x esp:%x\n", *(uint32_t *)(frame->esp - 4), *(uint32_t *)frame->esp);
    
    /* 原本trigger_frame是在用户栈esp-trigger_frameSize这个位置，但是由于调用了用户处理程序后，
    函数会把返回地址弹出，也就是esp+4，所以，需要通过esp-4才能获取到trigger_frame */
    trigger_frame_t *trigger_frame = (trigger_frame_t *)(frame->esp - 4);
    trigset_t oldset = trigger_frame->oldmask;
    
    triggers_t *trigger = current_task->triggers; 
    spin_lock_irq(&trigger->trig_lock);
    trigger->blocked = oldset;
    trigger_calc_left(trigger);
    spin_unlock_irq(&trigger->trig_lock);
    
    /* 还原之前的中断栈 */
    memcpy(frame, &trigger_frame->trap_frame, sizeof(trap_frame_t));
#ifdef DEBUG_TASK
    printk(KERN_DEBUG "sys_trigger_return: ret val 0x%x.\n", frame->eax);
#endif
    /* 会修改eax的值，返回eax的值 */
    return frame->eax;
}

void dump_task_kstack(thread_stack_t *kstack)
{
    printk(KERN_INFO "eip:%x func:%x arg:%x ebp:%x ebx:%x esi:%x edi:%x\n", 
    kstack->eip, kstack->function, kstack->arg, kstack->ebp, kstack->ebx, kstack->esi, kstack->edi);
}
