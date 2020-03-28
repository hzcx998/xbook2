#include <arch/interrupt.h>
#include <arch/task.h>
#include <xbook/task.h>
#include <xbook/schedule.h>
#include <xbook/process.h>
#include <xbook/vmspace.h>
#include <xbook/string.h>

/* 中断返回 */
static int copy_struct_and_kstack(task_t *child, task_t *parent)
{
    /* 复制task所在的页，里面有结构体，0级栈，还有返回地址等 */
    memcpy(child, parent, TASK_KSTACK_SIZE);

    /* 单独修改内容 */
    child->pid = fork_pid();
    child->elapsed_ticks = 0;
    child->state = TASK_READY;
    child->ticks = child->timeslice;
    child->parent_pid = parent->pid;
    /* 重新设置链表，在这里不使用ListDel，那样会删除父进程在队列中的情况
    所以这里就直接把队列指针设为NULL，后面会添加到链表中*/
    child->list.next = child->list.prev = NULL;
    child->global_list.next = child->global_list.prev = NULL;
    /* 设置内核栈位置 */
    child->kstack = (unsigned char *)((unsigned char *)child + TASK_KSTACK_SIZE - sizeof(trap_frame_t));
    /* 复制名字，在后面追加fork表明是一个fork的进程，用于测试 */
    //strcat(child->name, "_fork");
    dump_trap_frame((trap_frame_t *)child->kstack);
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
    
    uint32_t prog_vaddr = 0;
    uint32_t paddr;
    
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
                printk(KERN_ERR "copy_vm_mapping: GetFreePage for vaddr failed!\n");
        
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
    /* 复制页表内容，因为所有的东西都在里面 */
    if (copy_vm_mapping(child, parent))
        return -1;
    
    /* 复制VMSpace */
    if (copy_vm_vmspace(child, parent))
        return -1;
    
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
    printk(KERN_DEBUG "in do_usrmsg_fork now. parent %s-%x child %s-%x\n", 
            parent->name, parent, child->name, child);
*/
    /* 2.初始化任务的内存管理器 */
    if (proc_vmm_init(child)) {
        return -1;
    }

    /* 3.复制页表和虚拟内存空间 */
    if (copy_vm(child, parent))
        return -1;

    return 0;
}

/**
 * do_usrmsg_fork - 执行进程分支
 * 
 * 如果失败，应该回滚回收之前分配的内存
 * 创建一个和自己一样的进程
 * 返回-1则失败，返回0表示子进程自己，返回>0表示父进程
 */
int do_usrmsg_fork(umsg_t *msg)
{
    /* 保存之前状态并关闭中断 */
    msg->retval = 0; /* 默认返回0，表示是子进程 */
    
    /* 把当前任务当做父进程 */
    task_t *parent = current_task;
    printk(KERN_DEBUG "parent %s pid=%d prio=%d is forking now.\n", 
        parent->name, parent->pid, parent->priority);
    /* 为子进程分配空间 */
    task_t *child = kmalloc(TASK_KSTACK_SIZE);
    if (child == NULL) {
        printk(KERN_ERR "do_usrmsg_fork: kmalloc for child task failed!\n");
        msg->retval = -1;
        return -1;
    }
    /* 当前中断处于关闭中，并且父进程有页目录表 */
    ASSERT(parent->vmm != NULL);
    
    /* 复制进程 */
    if (copy_task(child, parent)) {
        printk(KERN_ERR "do_usrmsg_fork: copy task failed!\n");
        kfree(child);
        msg->retval = -1;
        return -1;
    }
    /* 把子进程添加到就绪队列和全局链表 */
    task_global_list_add(child);
    task_priority_queue_add_tail(child); /* 放到队首 */
    
    printk(KERN_DEBUG "task %s pid %d fork task %s pid %d\n", 
        parent->name, parent->pid, child->name, child->pid);
    
    /* 父进程消息返回进程pid */
    msg->retval = child->pid;

    /* fork成功 */
    return 0;
}