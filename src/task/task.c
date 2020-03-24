#include <arch/interrupt.h>
#include <arch/page.h>
#include <arch/general.h>
#include <arch/cpu.h>
#include <xbook/task.h>
#include <xbook/memops.h>
#include <xbook/string.h>
#include <xbook/assert.h>
#include <xbook/debug.h>
#include <xbook/schedule.h>
#include <xbook/spinlock.h>
#include <xbook/mutexlock.h>
#include <xbook/semaphore.h>
#include <xbook/synclock.h>
#include <xbook/fifobuf.h>
#include <xbook/fifoio.h>
#include <xbook/rwlock.h>

static pid_t next_pid;

/* 初始化链表头 */

// 全局队列链表，用来查找所有存在的任务
LIST_HEAD(task_global_list);

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

#if 0
/**  
 * roll_back_pid - 回滚一个pid
 */
static void roll_back_pid()
{
    --next_pid;
}
#endif

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

    task_vmm_init_page(page);
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
    init_task_lock_stack(local_stack, do_task_func, function, arg);
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

    /* no priority queue */
    task->prio_queue = NULL;

    task->next = NULL;
    
    /* task stack magic */
    task->stack_magic = TASK_STACK_MAGIC;
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
        task_vmm_active(task->vmm);
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

DEFINE_MUTEX_LOCK(pmutex);

DEFINE_SEMAPHORE(psema, 1);

DEFINE_SPIN_LOCK(pspin);

DEFINE_SYNC_LOCK(psync);

DEFINE_FIFO_IO(fifo_io, NULL, 0);
DEFINE_FIFO_BUF(fifo_buf, NULL, 0);

//#define PRINT_MUTEX
//#define PRINT_SEMA
//#define PRINT_SPIN
//#define PRINT_SYNC
//#define PRINT_FIFO
#define PRINT_RW_LOCK

fifo_buf_t *kfifo;

fifo_io_t *iofifo;

//rwlock_t rwlock;
DEFINE_RWLOCK_WR_FIRST(rwlock);

DEFINE_RWLOCK_RD_FIRST(rdfirst);
DEFINE_RWLOCK_WR_FIRST(wrfirst);
DEFINE_RWLOCK_RW_FAIR(rwfair);

int rwlock_int = 0;

#define FREQ 0X1000

int testA = 0, testB = 0;
void taskA(void *arg)
{
    //char *par = arg;
    int i = 0;
    int data = 0;
    while (1) {
        i++;
        testA++;
        if (i%FREQ == 0) {
            /* 写者 */
            #ifdef PRINT_RW_LOCK
            rwlock_wrlock(&rwlock);
            rwlock_int++;
            rwlock_wrunlock(&rwlock);            
            #endif

            #ifdef PRINT_FIFO
            data++;
            // fifo_buf_put(kfifo, (const unsigned char *  )"hello, first!", 13);

            fifo_io_put(iofifo, data);
            #endif

            #ifdef PRINT_SYNC
            sync_lock(&psync);
            #endif
            
            #ifdef PRINT_SPIN
            spin_lock(&pspin);
            #endif
            
            #ifdef PRINT_SEMA
            semaphore_down(&psema);
            #endif
            
            #ifdef PRINT_MUTEX
            mutex_lock(&pmutex);
            #endif
            //printk("<abcdefghabcdefabcdefghabcdefabcdefghabcdefabcdefghabcdefabcdefghabcdefabcdefghabcdef>\n");
            #ifdef PRINT_MUTEX
            mutex_unlock(&pmutex);
            #endif

            #ifdef PRINT_SEMA
            semaphore_up(&psema);
            
            #endif
            
            #ifdef PRINT_SPIN
            spin_unlock(&pspin);
            #endif
            
            #ifdef PRINT_SYNC
            sync_unlock(&psync);
            #endif
            
            //lockPrintk(par);
            
            //printk("A:%8x ", testA);
        }
        #ifdef PRINT_FIFO
            
        if (i%100 == 0)
            printk("\navali:%x\n", fifo_io_len(iofifo));
        #endif
        /*
        if (i > 0xf000000)
            kthread_exit(current_task);*/
    }
}

void taskB(void *arg)
{
    //char *par = arg;
    int i = 0;
    // log("hello\n");
    unsigned char buffer[16];
    int len;
    while (1) {
        i++;
        testB++;
        if (i%FREQ == 0) {
            /* 读者 */
            #ifdef PRINT_RW_LOCK
            rwlock_rdlock(&rwlock);
            printk("B:%x\n", rwlock_int);
            rwlock_rdunlock(&rwlock);            
            #endif
            
            #ifdef PRINT_FIFO
            
            /*memset(buffer, 0, 16);
            len = fifo_buf_get(kfifo, buffer, 13);
            printk("get buffer:%s len:%d\n", buffer, len);
            */
            unsigned char data = fifo_io_get(iofifo);
            printk("<%d>", data);
            #endif

            #ifdef PRINT_SYNC
            sync_lock(&psync);
            #endif
            
            #ifdef PRINT_SPIN
            spin_lock(&pspin);
            #endif
            
            #ifdef PRINT_SEMA
            semaphore_down(&psema);
            #endif
            
            #ifdef PRINT_MUTEX
            mutex_lock(&pmutex);
            #endif
            //printk("[123456781234561234567812345612345678123456123456781234561234567812345612345678123456]\n");
            #ifdef PRINT_MUTEX
            mutex_unlock(&pmutex);
            #endif
            
            #ifdef PRINT_SEMA
            semaphore_up(&psema);
            #endif
            
            #ifdef PRINT_SPIN
            spin_unlock(&pspin);
            #endif
            
            #ifdef PRINT_SYNC
            sync_unlock(&psync);
            #endif
            
            //SysMSleep(3000);
            //printk("B:%8x ", testB);
        }
        #ifdef PRINT_FIFO
            
        if (i%100 == 0)
            printk("\nlen:%x\n", fifo_io_len(iofifo));
        #endif
        /*
        if (i > 0xf000000)
            kthread_exit(current_task);*/
    }
}

void taskC(void *arg)
{
    //char *par = arg;
    int i = 0;
    // log("hello\n");
    while (1) {
        i++;
        if (i%FREQ == 0) {
            
            /* 读者 */
            #ifdef PRINT_RW_LOCK
            rwlock_rdlock(&rwlock);
            printk("C:%x\n", rwlock_int);
            rwlock_rdunlock(&rwlock);            
            #endif

            #ifdef PRINT_SYNC
            sync_lock(&psync);
            #endif
            
            #ifdef PRINT_SPIN
            spin_lock(&pspin);
            #endif
            
            #ifdef PRINT_SEMA
            semaphore_down(&psema);
            #endif
            
            #ifdef PRINT_MUTEX
            mutex_lock(&pmutex);
            #endif
            //printk("[~!@#$^&*()_+~!@#$^&*()_+~!@#$^&*()_+~!@#$^&*()_+~!@#$^&*()_+~!@#$^&*()_+~!@#$^&*()_+]\n");
            #ifdef PRINT_MUTEX
            mutex_unlock(&pmutex);
            #endif

            #ifdef PRINT_SEMA
            semaphore_up(&psema);
            #endif
            
            #ifdef PRINT_SPIN
            spin_unlock(&pspin);
            #endif
            
            #ifdef PRINT_SYNC
            sync_unlock(&psync);
            #endif
            
            //SysMSleep(3000);
            //printk("B:%8x ", testB);
        }
        /*
        if (i > 0xf000000)
            kthread_exit(current_task);*/
    }
}

void taskD(void *arg)
{
    //char *par = arg;
    int i = 0;
    // log("hello\n");
    while (1) {
        i++;
        if (i%FREQ == 0) {
            /* 写者 */
            #ifdef PRINT_RW_LOCK
            rwlock_wrlock(&rwlock);
            rwlock_int++;
            rwlock_wrunlock(&rwlock);            
            #endif

            #ifdef PRINT_SYNC
            sync_lock(&psync);
            #endif
            
            #ifdef PRINT_SPIN
            spin_lock(&pspin);
            #endif
            
            #ifdef PRINT_SEMA
            semaphore_down(&psema);
            #endif
            
            #ifdef PRINT_MUTEX
            mutex_lock(&pmutex);
            #endif
            //printk("[------------------------------------------------------------------------------------]\n");
            #ifdef PRINT_MUTEX
            mutex_unlock(&pmutex);
            #endif

            #ifdef PRINT_SEMA
            semaphore_up(&psema);
            #endif
            
            #ifdef PRINT_SPIN
            spin_unlock(&pspin);
            #endif
            #ifdef PRINT_SYNC
            sync_unlock(&psync);
            #endif
            
            //SysMSleep(3000);
            //printk("B:%8x ", testB);
        }
        /*
        if (i > 0xf000000)
            kthread_exit(current_task);*/
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
    
    /* 调度到其它任务，直到又重新被调度 */
    schedule();

    printk("run idle");
    int i = 0;

    /* idle线程 */
	while (1) {
        i++;
		/* 进程默认处于阻塞状态，如果被唤醒就会执行后面的操作，
		知道再次被阻塞 */
		//printk("*%d",i);
        /* 打开中断 */
		enable_intr();
		/* 执行cpu停机 */
		//cpu_idle();
        if (i % 0x500000 == 0) {
            printk("idle\n");
            //kthread_start("test", 1, taskA, "NULL");
        }
        if (i % 0xf00000 == 0) {
            printk("kthread a\n");
            kthread_start("test", 1, taskA, "NULL");
        }

        if (i % 0xf00000 == 0) {
            printk("kthread b\n");
            kthread_start("test2", 1, taskB, "NULL");
        }
	};
}

/**
 * init_tasks - 初始化多任务环境
 */
void init_tasks()
{
    init_schedule();

    next_pid = 0;
    
    make_main_task();
    
    /*kfifo = fifo_buf_alloc(128);
    if (kfifo == NULL)
        printk(KERN_ERR "alloc fifo buf failed!\n");
    */
    iofifo = fifo_io_alloc(128);
    if (iofifo == NULL)
        printk(KERN_ERR "alloc fifo buf failed!\n");
    
    //rwlock_init(&rwlock, RWLOCK_RW_FAIR);

    /* 有可能做测试阻塞main线程，那么就没有线程，
    在切换任务的时候就会出错，所以这里创建一个测试线程 */
    kthread_start("test", 1, taskA, "NULL");
    kthread_start("test2", 1, taskB, "NULL");
    kthread_start("test3", 1, taskC, "NULL");
    kthread_start("test4", 1, taskD, "NULL");
    
}
