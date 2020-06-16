#include <arch/interrupt.h>
#include <arch/task.h>
#include <xbook/task.h>
#include <xbook/schedule.h>
#include <xbook/process.h>
#include <xbook/vmspace.h>
#include <xbook/string.h>

#define DEBUG_LOCAL 0

/**
 * 在多线程中，fork只会把调用者线程复制给子进程，而其它线程就会“蒸发”。
 * 
 */
/* 中断返回 */
static int copy_struct_and_kstack(task_t *child, task_t *parent)
{
    /* 复制task所在的页，里面有结构体，0级栈，还有返回地址等 */
    memcpy(child, parent, TASK_KSTACK_SIZE);

    /* 单独修改内容 */
    child->pid = fork_pid();
    child->tgid = child->pid;      /* 生成的进程是主线程 */
    child->state = TASK_READY;
    child->parent_pid = parent->pid;
    /* 重新设置链表，在这里不使用list_del，那样会删除父进程在队列中的情况
    所以这里就直接把队列指针设为NULL，后面会添加到链表中*/
    child->list.next = child->list.prev = NULL;
    child->global_list.next = child->global_list.prev = NULL;
    /* 设置内核栈位置 */
    child->kstack = (unsigned char *)((unsigned char *)child + TASK_KSTACK_SIZE - sizeof(trap_frame_t));
    
    /* 复制名字，在后面追加fork表明是一个fork的进程，用于测试 */
    //strcat(child->name, "_fork");
    //dump_trap_frame((trap_frame_t *)child->kstack);
    return 0;
}

/* 复制vm结构的内容 */
static int copy_vm_struct(task_t *child, task_t *parent)
{
    /* 由于初始化vmm时已经初始化了page_storege，因此这里不复制，只复制空间的范围 */
    child->vmm->code_start = parent->vmm->code_start;
    child->vmm->data_start = parent->vmm->data_start;
    child->vmm->heap_start = parent->vmm->heap_start;
    child->vmm->map_start = parent->vmm->map_start;
    child->vmm->share_start = parent->vmm->share_start;
    child->vmm->stack_start = parent->vmm->stack_start;
    child->vmm->arg_start = parent->vmm->arg_start;
    child->vmm->env_start = parent->vmm->env_start;
    child->vmm->code_end = parent->vmm->code_end;
    child->vmm->data_end = parent->vmm->data_end;
    child->vmm->heap_end = parent->vmm->heap_end;
    child->vmm->map_end = parent->vmm->map_end;
    child->vmm->share_end = parent->vmm->share_end;
    child->vmm->stack_end = parent->vmm->stack_end;
    child->vmm->arg_end = parent->vmm->arg_end;
    child->vmm->env_end = parent->vmm->env_end;
    return 0;
}

/* 复制进程虚拟内存的映射 */
static int copy_vm_mapping(task_t *child, task_t *parent)
{
    /* 开始内存的复制 */
    void *buf = kmalloc(PAGE_SIZE);
    if (buf == NULL) {
        printk(KERN_ERR "copy_vm_mapping: kmalloc buf for data transform failed!\n");
        return -1;
    }
    /* 获取父目录的虚拟空间 */
    vmspace_t *space = parent->vmm->vmspace_head;
    
    unsigned long prog_vaddr = 0;
    unsigned long paddr;
    
    /* 当空间不为空时就一直获取 */
    while (space != NULL) {
        /* 获取空间最开始地址 */
        prog_vaddr = space->start;
        // printk(KERN_DEBUG "the space %x start %x end %x\n", space, space->start, space->end);
        /* 在空间中进行复制 */
        while (prog_vaddr < space->end) {
            /* 1.将进程空间中的数据复制到内核空间，切换页表后，
            还能访问到父进程中的数据 */
            memcpy(buf, (void *)prog_vaddr, PAGE_SIZE);

            /* 2.切换进程空间 */
            vmm_active(child->vmm);

            /* 3.映射虚拟地址 */
            // 分配一个物理页
            paddr = alloc_page();
            if (!paddr) {
                printk(KERN_ERR "copy_vm_mapping: alloc_page for vaddr failed!\n");
        
                /* 激活父进程并返回 */
                vmm_active(parent->vmm);

                return -1;
            }
            // 根据空间的保护来设定页属性
            page_link(prog_vaddr, paddr, PG_RW_W | PG_US_U);

            /* 4.从内核复制数据到进程 */
            memcpy((void *)prog_vaddr, buf, PAGE_SIZE);

            /* 5.恢复父进程内存空间 */
            vmm_active(parent->vmm);
            // printk(KERN_DEBUG "copy at virtual address %x\n", prog_vaddr);
            /* 指向下一个页 */
            prog_vaddr += PAGE_SIZE;
        }
        /* 指向下一个空间 */
        space = space->next;
    }
    kfree(buf);
    return 0; 
}

static int copy_vm_vmspace(task_t *child, task_t *parent)
{
    /* 空间头 */
    vmspace_t *tail = NULL;
    /* 指向父任务的空间 */
    vmspace_t *p = parent->vmm->vmspace_head;
    while (p != NULL) {
        /* 分配一个空间 */
        vmspace_t *space = vmspace_alloc();
        if (space == NULL) {
            printk(KERN_ERR "copy_vm_vmspace: kmalloc for space failed!\n");
            return -1;
        }
            
        /* 复制空间信息 */
        *space = *p;
        /* 把下一个空间置空，后面加入链表 */
        space->next = NULL;

        /* 如果空间表头是空，那么就让空间表头指向第一个space */
        if (tail == NULL)
            child->vmm->vmspace_head = space;    
        else 
            tail->next = space; /* 让空间位于子任务的空间表的最后面 */

        /* 让空间头指向新添加的空间 */
        tail = space;

        /* 获取下一个空间 */
        p = p->next;
    }
    /* 打印子进程space */
#if 0
    p = child->vmm->vmspace_head;
    while (p != NULL) {
        printk(KERN_DEBUG "[child] space %x start %x end %x flags %x\n",
                p, p->start, p->end, p->flags);
        p = p->next;
    }
#endif
    return 0;
}

static int copy_vm(task_t *child, task_t *parent)
{
    /* 复制vmm管理器结构 */
    if (copy_vm_struct(child, parent))
        return -1;

    /* 复制页表内容，因为所有的东西都在里面 */
    if (copy_vm_mapping(child, parent))
        return -1;
    
    /* 复制VMSpace */
    if (copy_vm_vmspace(child, parent))
        return -1;
    
    return 0;
}

static int copy_trigger(task_t *child, task_t *parent)
{
    child->triggers = kmalloc(sizeof(triggers_t));
    if (child->triggers == NULL)
        return -1;
    /* 复制触发器结构 */
    *child->triggers = *parent->triggers;
    return 0;
}

static int copy_res(task_t *child, task_t *parent)
{
    child->res = kmalloc(sizeof(resource_t));
    if (child->res == NULL)
        return -1;
    memset(child->res, 0, sizeof(resource_t));
    resource_init(child->res);
    /* 更新资源引用 */
    //dump_resource(parent->res);
    resource_copy(child->res, parent->res);
    return 0;
}

/**
 * copy_pthread_desc - 复制线程描述结构
 * 
 */
static int copy_pthread_desc(task_t *child, task_t *parent)
{
    /* 父进程是单线程就没有线程描述 */
    if (parent->pthread != NULL) { 
        /* 由于复制后，只有一个线程，所以这里就直接初始化一个新的，而不是复制 */
        if (proc_pthread_init(child))
            return -1;
    }
    return 0;
}

static int bulid_child_stack(task_t *child)
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
 * copy_task - 拷贝父进程的资源给子进程
 */
static int copy_task(task_t *child, task_t *parent)
{
    /* 1.复制任务结构体和内核栈道子进程 */
    if (copy_struct_and_kstack(child, parent))
        return -1;
    /*
    printk(KERN_DEBUG "copy_task: parent %s-%x child %s-%x\n", 
            parent->name, parent, child->name, child);
*/
    /* 2.初始化任务的内存管理器 */
    if (proc_vmm_init(child)) {
        return -1;
    }

    /* 3.复制页表和虚拟内存空间 */
    if (copy_vm(child, parent))
        return -1;
    
    /* 4.复制触发器 */
    if (copy_trigger(child, parent))
        return -1;

    /* 5.复制资源 */
    if (copy_res(child, parent))
        return -1;
        
    /* 6.复制线程描述 */
    if (copy_pthread_desc(child, parent))
        return -1;

    bulid_child_stack(child);
    // printk(KERN_DEBUG "child heap is [%x,%x]\n", child->vmm->heap_start, child->vmm->heap_end);
    return 0;
}

/**
 * proc_fork - 执行进程分支
 * 
 * 如果失败，应该回滚回收之前分配的内存
 * 创建一个和自己一样的进程
 * 返回-1则失败，返回0表示子进程自己，返回>0表示父进程
 */
int sys_fork()
{
    /* 把当前任务当做父进程 */
    task_t *parent = current_task;
#if DEBUG_LOCAL == 1
    printk(KERN_DEBUG "%s: parent %s pid=%d prio=%d is forking now.\n", 
        __func__, parent->name, parent->pid, parent->priority);
#endif    
    /* 为子进程分配空间 */
    task_t *child = kmalloc(TASK_KSTACK_SIZE);
    if (child == NULL) {
        printk(KERN_ERR "do_usrmsg_fork: kmalloc for child task failed!\n");
        return -1;
    }
    /* 当前中断处于关闭中，并且父进程有页目录表 */
    ASSERT(parent->vmm != NULL);
    unsigned long flags;
    save_intr(flags);

    /* 复制进程 */
    if (copy_task(child, parent)) {
        printk(KERN_ERR "do_usrmsg_fork: copy task failed!\n");
        kfree(child);
        return -1;
    }
    
    /* 把子进程添加到就绪队列和全局链表 */
    task_global_list_add(child);
    task_priority_queue_add_tail(child); /* 放到队首 */


#if DEBUG_LOCAL == 1
    printk(KERN_DEBUG "%s: task %s pid %d fork task %s pid %d\n", 
        __func__, parent->name, parent->pid, child->name, child->pid);
#endif
    restore_intr(flags);
    
    /* 父进程返回子进程pid */
    return child->pid;
}