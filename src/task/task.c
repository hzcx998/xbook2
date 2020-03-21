#include <arch/interrupt.h>
#include <arch/page.h>
#include <arch/general.h>
#include <xbook/task.h>
#include <xbook/memops.h>
#include <xbook/string.h>
#include <xbook/assert.h>
#include <xbook/debug.h>
#include <xbook/schedule.h>

//extern void UpdateTssInfo(task_t *task);

static pid_t next_pid;

/* 初始化链表头 */

// 就绪队列链表
// 全局队列链表，用来查找所有存在的任务
LIST_HEAD(task_global_list);

/* 优先级队列链表 */
list_t task_priority_queue[MAX_PRIORITY_NR];

/* idle任务 */
task_t *task_idle;
/**
 * do_task_func - 执行内核线程
 * @function: 要执行的线程
 * @arg: 参数
 * 
 * 改变当前的执行流，去执行我们选择的内核线程
 */
static void do_task_func(task_func_t *function, void *arg)
{
    enable_intr();  /* enable interrupt to make sure func will do */
    function(arg);  /* call func(arg) */
}

/**  
 * new_pid - 分配一个pid
 */
static pid_t new_pid()
{
    return next_pid++;
}

/**  
 * roll_back_pid - 回滚一个pid
 */
static void roll_back_pid()
{
    --next_pid;
}

/**
 * task_create_page_storage - 创建页储存   
 */
unsigned long *task_create_page_storage()
{
    // 分配一个页来当作页目录
    unsigned long *page = kmalloc(PAGE_SIZE);

    if (!page) {
        printk("kmalloc for task_create_page_storage failed!\n");
        return NULL;
    }
    memset(page, 0, PAGE_SIZE);

    __task_vmm_init_page(page);
    return page;
}

/**
 * make_task_stack - 创建一个线程
 * @task: 线程结构体
 * @function: 要去执行的函数
 * @arg: 参数
 */
static void make_task_stack(task_t *task, task_func_t function, void *arg)
{
    /* 预留中断栈 */
    task->kstack -= sizeof(trap_frame_t);

    /* 预留线程栈 */
    task->kstack -= sizeof(task_local_stack_t);
    /* 填写本地栈信息 */
    task_local_stack_t *local_stack = (task_local_stack_t *)task->kstack;

    // 在do_task_func中去改变执行流，从而可以传递一个参数
    __init_task_lock_stack(local_stack, do_task_func, function, arg);
}

/**
 * task_init - 初始化线程
 * @task: 线程结构地址
 * @name: 线程的名字
 * @priority: 线程优先级
 */
static void task_init(task_t *task, char *name, int priority)
{
    memset(task, 0, sizeof(task_t));
    // 复制名字
    strcpy(task->name, name);

    task->state = TASK_READY;
    
    // 修复优先级
    if (priority < 0)
        priority = 0;
    if (priority > MAX_PRIORITY_NR - 1)
        priority = MAX_PRIORITY_NR - 1;

    task->priority = priority;
    task->timeslice = 3;  /* 时间片大小默认值 */
    task->ticks = task->timeslice;
    
    task->elapsed_ticks = 0;
    
    /* init with NULL */
    task->vmm = kmalloc(sizeof(vmm_t));
    if (task->vmm == NULL) {
        panic("error: task_init: kmalloc for vmm failed!\n");
    }
    task->vmm->page_storage = NULL;

    /* get a new pid */
    task->pid = new_pid();
    task->parent_pid = -1;
    task->exit_state = -1;

    // set kernel stack as the top of task mem struct
    task->kstack = (unsigned char *)(((unsigned long )task) + TASK_KSTACK_SIZE);

    /* task stack magic */
    task->stack_magic = TASK_STACK_MAGIC;
}

/**
 * find_task_by_pid - 初始化任务的内存管理
 * @task: 任务
 */
task_t *find_task_by_pid(pid_t pid)
{
    task_t *task;
    /* 关闭中断 */
    unsigned long flags;
    save_intr(flags);
    list_for_each_owner(task, &task_global_list, global_list) {
        if (task->pid == pid) {
            restore_intr(flags);
            return task;
        }
    }
    restore_intr(flags);
    return NULL;
}

/**
 * task_priority_queue_add_tail - 把任务添加到特权级队列末尾
 * @task: 任务
 *  
 */
void task_priority_queue_add_tail(task_t *task)
{
    /* 添加到相应的优先级队列 */
    ASSERT(!list_find(&task->list, &task_priority_queue[task->priority]));
    // 添加到就绪队列
    list_add_tail(&task->list, &task_priority_queue[task->priority]);
}

/**
 * task_priority_queue_add_head - 把任务添加到特权级队列头部
 * @task: 任务
 * 
 */
void task_priority_queue_add_head(task_t *task)
{
    /* 添加到相应的优先级队列 */
    ASSERT(!list_find(&task->list, &task_priority_queue[task->priority]));
    // 添加到就绪队列
    list_add(&task->list, &task_priority_queue[task->priority]);
}

/**
 * task_global_list_add - 把任务添加到全局队列
 * @task: 任务
 */
void task_global_list_add(task_t *task)
{
    // 保证不存在于链表中
    ASSERT(!list_find(&task->global_list, &task_global_list));
    // 添加到全局队列
    list_add_tail(&task->global_list, &task_global_list);
}

/**
 * is_task_in_priority_queue - 把任务添加到全局队列
 * @task: 任务
 */
int is_task_in_priority_queue(task_t *task)
{
    int i;
    for (i = 0; i < MAX_PRIORITY_NR; i++) {
        if (list_find(&task->list, &task_priority_queue[i])) {
            return 1;
        }
    }
    return 0;
}


/**
 * is_all_priority_queue_empty - 判断优先级队列是否为空
 */
int is_all_priority_queue_empty()
{
    int i;
    for (i = 0; i < MAX_PRIORITY_NR; i++) {
        if (!list_empty(&task_priority_queue[i])) {
            return 0;
        }
    }
    return 1;
}

/**
 * kthread_start - 开始一个线程
 * @name: 线程的名字
 * @func: 线程入口
 * @arg: 线程参数
 */
task_t *kthread_start(char *name, int priority, task_func_t func, void *arg)
{
    // 创建一个新的线程结构体
    task_t *task = (task_t *) kmalloc(TASK_KSTACK_SIZE);
    
    if (!task)
        return NULL;
    
    // 初始化线程
    task_init(task, name, priority);
    
    //printk("alloc a task at %x\n", task);
    // 创建一个线程
    make_task_stack(task, func, arg);

    /* 操作链表时关闭中断，结束后恢复之前状态 */
    unsigned long flags;
    save_intr(flags);

    task_global_list_add(task);
    task_priority_queue_add_tail(task);
    
    restore_intr(flags);
    return task;
}

/**
 * make_main_task - 为内核主线程设定身份
 * 
 * 内核主线程就是从boot到现在的执行流。到最后会演变成idle
 * 在这里，我们需要给与它一个身份，他才可以进行多线程调度
 */
static void make_main_task()
{
    // 当前运行的就是主线程
    task_idle = current_task;
    
    /* 最开始设置为最佳优先级，这样才可以往后面运行。直到运行到最后，就变成IDLE优先级 */
    task_init(task_idle, "idle", TASK_PRIO_BEST);

    /* 设置为运行中 */
    task_idle->state = TASK_RUNNING;

    task_global_list_add(task_idle);
}

/**
 * task_activate - 激活任务
 * @task: 要激活的任务
 */
void task_activate(task_t *task)
{
    /* 任务不能为空 */
    ASSERT(task != NULL);
    
    /* 设置为运行状态 */
    task->state = TASK_RUNNING;
    /* 激活任务的页目录表 */
    if (task->vmm)
        task_vmm_active(task);
}

/**
 * task_block - 把任务阻塞
 */
void task_block(task_state_t state)
{
    /*
    state有4种状态，分别是TASK_BLOCKED, TASK_WAITING, TASK_STOPPED, TASK_ZOMBIE
    它们不能被调度
    */
    ASSERT ((state == TASK_BLOCKED) || 
            (state == TASK_WAITING) || 
            (state == TASK_STOPPED) ||
            (state == TASK_ZOMBIE));
    // 先关闭中断，并且保存中断状态
    unsigned long flags;
    save_intr(flags);

    // 改变状态
    task_t *current = current_task;
    //printk(PART_TIP "task %s blocked with status %d\n", current->name, state);
    current->state = state;
    
    // 调度到其它任务
    schedule();

    // 恢复之前的状态
    restore_intr(flags);
}
/**
 * task_unblock - 解除任务阻塞
 * @task: 要解除的任务
 */
void task_unblock(task_t *task)
{
    /*
    state有2种状态，分别是TASK_BLOCKED, TASK_WAITING
    只有它们能被唤醒, TASK_ZOMBIE只能阻塞，不能被唤醒
    */
    ASSERT((task->state == TASK_BLOCKED) || 
        (task->state == TASK_WAITING) ||
        (task->state == TASK_STOPPED));
    
    // 先关闭中断，并且保存中断状态
    unsigned long flags;
    save_intr(flags);
    // 没有就绪才能够唤醒，并且就绪
    if (task->state != TASK_READY) {
        // 保证没有在就绪队列中
        ASSERT(!is_task_in_priority_queue(task));
        // 已经就绪是不能再次就绪的
        if (is_task_in_priority_queue(task)) {
            panic("TaskUnblock: task has already in ready list!\n");
        }
        // 处于就绪状态
        task->state = TASK_READY;
        // 把任务放在最前面，让它快速得到调度
        task_priority_queue_add_head(task);
    }
    restore_intr(flags);
}

/**
 * print_task - 打印所有任务
 */
void print_task()
{
    printk("\n----Task----\n");
    task_t *task;
    list_for_each_owner(task, &task_global_list, global_list) {
        printk("name %s pid %d ppid %d status %d\n", 
            task->name, task->pid, task->parent_pid,  task->state);
    }
}

int testA = 0, testB = 0;
void taskA(void *arg)
{
    //char *par = arg;
    int i = 0;
    while (1) {
        i++;
        testA++;
        if (i%0xf0000 == 0) {
            
            //lockPrintk("<abcdefghabcdef> ");

            //lockPrintk(par);
            
            //printk("A:%x ", testA);
        }

        if (i > 0xf000000)
            kthread_exit(current_task);
    }
}

void taskB(void *arg)
{
    //char *par = arg;
    int i = 0;
    // log("hello\n");
    while (1) {
        i++;
        testB++;
        if (i%0xf0000 == 0) {
            
            //lockPrintk("[12345678123456] ");
            
            //SysMSleep(3000);
            //printk("B:%x ", testB);
        }

        if (i > 0xf00000)
            kthread_exit(current_task);
    }
}

void dump_task(task_t *task)
{
    printk("----Task----\n");
    printk("name:%s pid:%d parent pid:%d state:%d\n", task->name, task->pid, task->parent_pid, task->state);
    printk("vmm->vm_frame:%x priority:%d ticks:%d elapsed ticks:%d\n", task->vmm->page_storage, task->priority, task->ticks, task->elapsed_ticks);
    printk("exit code:%d stack magic:%d\n", task->exit_state, task->stack_magic);
}

/**
 * kernel_pause - 内核进入暂停
 * 
 * 在初始化的最后调用，当前任务演变成idle任务，等待随时调动
 */
void kernel_pause()
{
    /* 设置特权级为最低，变成阻塞，当没有其它任务运行时，就会唤醒它 */
    task_idle->state = TASK_BLOCKED;
    task_idle->priority = TASK_PRIO_IDLE;
    /* 然后设置成最低特权级 */
    task_priority_queue_add_head(task_idle);
    /* 调度到其它任务，直到又重新被调度 */
    schedule();

    printk("wake up idle");
    /* idle线程 */
	while (1) {
		/* 进程默认处于阻塞状态，如果被唤醒就会执行后面的操作，
		知道再次被阻塞 */
		
        /* 打开中断 */
		enable_intr();
		/* 执行cpu停机 */
		__cpu_idle();
	};
}

/**
 * init_tasks - 初始化多任务环境
 */
void init_tasks()
{

    /* 初始化特权队列 */
    int i;
    for (i = 0; i < MAX_PRIORITY_NR; i++) {
        INIT_LIST_HEAD(&task_priority_queue[i]);
    }

    next_pid = 0;
    
    make_main_task();
   
    /* 有可能做测试阻塞main线程，那么就没有线程，
    在切换任务的时候就会出错，所以这里创建一个测试线程 */
    kthread_start("test", 1, taskA, "NULL");
    kthread_start("test2", 1, taskB, "NULL");

}
