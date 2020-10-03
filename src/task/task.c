#include <arch/interrupt.h>
#include <arch/page.h>
#include <arch/cpu.h>
#include <arch/task.h>
#include <arch/pmem.h>
#include <xbook/task.h>
#include <string.h>
#include <string.h>
#include <assert.h>
#include <xbook/debug.h>
#include <xbook/schedule.h>
#include <xbook/spinlock.h>
#include <xbook/mutexlock.h>
#include <xbook/semaphore.h>
#include <xbook/synclock.h>
#include <xbook/fifobuf.h>
#include <xbook/fifoio.h>
#include <xbook/rwlock.h>
#include <xbook/vmm.h>
#include <xbook/waitque.h>
#include <xbook/rawblock.h>
#include <xbook/process.h>
#include <fsal/fsal.h>
#include <math.h>

static pid_t next_pid;

/* 初始化链表头 */

// 全局队列链表，用来查找所有存在的任务
LIST_HEAD(task_global_list);

/* idle任务 */
task_t *task_kmain;

/**  
 * new_pid - 分配一个pid
 */
static pid_t new_pid()
{
    return next_pid++;
}

pid_t fork_pid()
{
    return new_pid();
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
 * task_init - 初始化线程
 * @task: 线程结构地址
 * @name: 线程的名字
 * @priority: 线程优先级
 */
void task_init(task_t *task, char *name, int priority)
{
    memset(task, 0, sizeof(task_t));
    // 复制名字
    strcpy(task->name, name);

    task->state = TASK_READY;
    
    spinlock_init(&task->lock);
    // 修复优先级
    if (priority < 0)
        priority = 0;
    if (priority > MAX_PRIORITY_NR - 1)
        priority = MAX_PRIORITY_NR - 1;

    task->priority = priority;
    task->timeslice = 3;  /* 时间片大小默认值 */
    task->ticks = task->timeslice;  /* timeslice -> ticks */
    task->elapsed_ticks = 0;    /* 总运行时间 */
    
    /* init with NULL */
    task->vmm = NULL;
    
    /* get a new pid */
    task->pid = new_pid();
    task->tgid = task->pid; /* 默认都是主线程，需要的时候修改 */
    task->parent_pid = -1;  /* no parent */
    task->exit_status = 0;  /* no status */

    // set kernel stack as the top of task mem struct
    task->kstack = (unsigned char *)(((unsigned long )task) + TASK_KSTACK_SIZE);

    /* no priority queue */
    //task->prio_queue = NULL;
    task->flags = 0;
    
    /* no triger */
    task->triggers = NULL;
    
    /* no timer */
    task->sleep_timer = NULL;
    
    /* set alarm */
    alarm_init(&task->alarm);

    /* set errno for user thread */
    task->errno = 0;

    /* 用户线程 */
    task->pthread = NULL;

    task->fileman = NULL;
    
    task->gmsgpool = NULL;
    
    /* task stack magic */
    task->stack_magic = TASK_STACK_MAGIC;
}

void task_free(task_t *task)
{
    /* remove from global list */
    list_del(&task->global_list);
    kfree(task);
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

void task_set_timeslice(task_t *task, uint32_t timeslice)
{
    if (task) {
        if (timeslice < TASK_MIN_TIMESLICE)
            timeslice = TASK_MIN_TIMESLICE;
        if (timeslice > TASK_MAX_TIMESLICE)
            timeslice = TASK_MAX_TIMESLICE;
        spin_lock(&task->lock);
        task->timeslice = timeslice;
        spin_unlock(&task->lock);
        
    }
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
task_t *kthread_start(char *name, int priority, task_func_t *func, void *arg)
{
    // 创建一个新的线程结构体
    task_t *task = (task_t *) kmalloc(TASK_KSTACK_SIZE);
    
    if (!task)
        return NULL;
    
    // 初始化线程
    task_init(task, name, priority);
    
    /* 创建文件描述表 */
    if (fs_fd_init(task) < 0) {
        kfree(task);
        return NULL;
    }

    //printk("alloc a task at %x\n", task);
    // 创建一个线程
    make_task_stack(task, func, arg);

    /* 操作链表时关闭中断，结束后恢复之前状态 */
    unsigned long flags;
    save_intr(flags);
    
    task_global_list_add(task);
    sched_unit_t *su = sched_get_unit();
    task_priority_queue_add_tail(su, task);
    
    restore_intr(flags);
    return task;
}

/**
 * task_activate - 激活任务
 * @task: 要激活的任务
 */
void task_activate(task_t *task)
{
    /* 任务不能为空 */
    ASSERT(task != NULL);
    spin_lock(&task->lock);
    /* 设置为运行状态 */
    task->state = TASK_RUNNING;
    spin_unlock(&task->lock);
    //printk("vmm:%x\n", task->vmm);
    /* 激活任务虚拟内存 */
    vmm_active(task->vmm);
}

/**
 * task_block - 把任务阻塞
 */
void task_block(task_state_t state)
{
    // 先关闭中断，并且保存中断状态
    unsigned long flags;
    save_intr(flags);

    /*
    state有4种状态，分别是TASK_BLOCKED, TASK_WAITING, TASK_STOPPED, TASK_ZOMBIE
    它们不能被调度
    */
    ASSERT ((state == TASK_BLOCKED) || 
            (state == TASK_WAITING) || 
            (state == TASK_STOPPED) ||
            (state == TASK_HANGING) ||
            (state == TASK_ZOMBIE));
    
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
    // 先关闭中断，并且保存中断状态
    unsigned long flags;
    save_intr(flags);
#if 1
    if (!((task->state == TASK_BLOCKED) || 
        (task->state == TASK_WAITING) ||
        (task->state == TASK_STOPPED))) {
        panic("task_unblock: task name=%s pid=%d state=%d\n", task->name, task->pid, task->state);
    }
#endif    
    /*
    state有3种状态，分别是TASK_BLOCKED, TASK_WAITING
    只有它们能被唤醒, TASK_ZOMBIE只能阻塞，不能被唤醒
    */
    /*ASSERT((task->state == TASK_BLOCKED) || 
        (task->state == TASK_WAITING) ||
        (task->state == TASK_STOPPED));
    */
    if (task->state != TASK_READY) {
        sched_unit_t *su = sched_get_unit();
        // 保证没有在就绪队列中
        ASSERT(!is_task_in_priority_queue(su, task));
        // 已经就绪是不能再次就绪的
        if (is_task_in_priority_queue(su, task)) {
            panic("TaskUnblock: task has already in ready list!\n");
        }
        // 处于就绪状态
        task->state = TASK_READY;
        // 把任务放在最前面，让它快速得到调度
        task_priority_queue_add_head(su, task);
    }
    
    restore_intr(flags);
    /* 由于现在状态变成就绪，并且在队列里面，所以下次切换任务时就会切换到任务，并且往后面运行 */
}

/**
 * task_yeild - 主动让出cpu
 * 
 */
void task_yeild()
{
    // 先关闭中断，并且保存中断状态
    unsigned long flags;
    save_intr(flags);
    current_task->state = TASK_READY; /* 设置为就绪状态 */
    schedule(); /* 调度到其它任务 */
    restore_intr(flags);
}

/**
 * create_kmain_thread - 创建内核主线程
 * 
 * 内核主线程就是从boot到现在的执行流。到最后会演变成idle
 * 在这里，我们需要给与它一个身份，他才可以进行多线程调度
 */
static void create_kmain_thread()
{
    // 当前运行的就是主线程
    task_kmain = (task_t *) KERNEL_STATCK_BOTTOM;
    sched_unit_t *su = sched_get_unit();
    su->cur = task_kmain;   /* 设置当前任务 */

    /* 最开始设置为最佳优先级，这样才可以往后面运行。直到运行到最后，就变成IDLE优先级 */
    task_init(task_kmain, "kmain", TASK_PRIO_BEST);

    if (fs_fd_init(task_kmain) < 0) {
        panic("init kmain fs fd failed!\n");
    }
    
    /* 设置为运行中 */
    task_kmain->state = TASK_RUNNING;
    task_global_list_add(task_kmain); /* 添加到全局任务 */
}

/**
 * sys_get_pid - 获取任务id
 */
pid_t sys_get_pid()
{
    /* 
    当调用者为进程时，tgid=pid
    当调用者为线程时，tgid=master process pid
    也就是说，线程返回的是主线程（进程）的pid
     */
    return current_task->tgid;
}

/**
 * sys_get_ppid - 获取任务父进程id
 */
pid_t sys_get_ppid()
{
    return current_task->parent_pid;
}

/**
 * sys_get_tid - 获取线程id
 */
pid_t sys_get_tid()
{
    /* 由于最小粒度是线程，所以，pid->线程id。 */
    return current_task->pid;
}

/**
 * print_task - 打印所有任务
 */
void print_task()
{
    printk("\n----Task----\n");
    task_t *task;
    list_for_each_owner(task, &task_global_list, global_list) {
        printk("name %s pid %d ppid %d state %d\n", 
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
//#define PRINT_RW_LOCK
#define PRINT_TEST 0

fifo_buf_t *kfifo;

fifo_io_t *iofifo;

//rwlock_t rwlock;
DEFINE_RWLOCK_WR_FIRST(rwlock);

int rwlock_int = 0;

#define FREQ 0X10000

int testA = 0, testB = 0;
void taskA(void *arg)
{
    //char *par = arg;
    int i = 0;
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
            #ifndef PRINT_RW_LOCK

#if PRINT_TEST == 1
            printk("<abcdefghabcdefabcdefghabcdefabcdefghabcdefabcdefghabcdefabcdefghabcdefabcdefghabcdef>\n");
#endif
            #endif
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
            #ifndef PRINT_RW_LOCK
#if PRINT_TEST == 1
            printk("[123456781234561234567812345612345678123456123456781234561234567812345612345678123456]\n");
#endif
            #endif
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
            #ifndef PRINT_RW_LOCK
#if PRINT_TEST == 1
            printk("[~!@#$^&*()_+~!@#$^&*()_+~!@#$^&*()_+~!@#$^&*()_+~!@#$^&*()_+~!@#$^&*()_+~!@#$^&*()_+]\n");
#endif
            #endif
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


/**
 * sys_tstate - 获取任务状态
 * @ts: 任务状态
 * @idx: 索引
 */
int sys_tstate(tstate_t *ts, unsigned int *idx)
{
    if (ts == NULL)
        return -1;

    /* 如果是0，就从第一个 */
    int n = 0;
    task_t *task;
    list_for_each_owner (task, &task_global_list, global_list) {
        /* 找到一致的索引 */
        if (n == *idx) {
            /* 复制数据信息 */
            ts->ts_pid = task->pid;
            ts->ts_ppid = task->parent_pid;
            ts->ts_tgid = task->tgid;
            ts->ts_state = task->state;
            ts->ts_priority = task->priority;
            ts->ts_timeslice = task->timeslice;
            ts->ts_runticks = task->elapsed_ticks;
            memset(ts->ts_name, 0, PROC_NAME_LEN);
            strcpy(ts->ts_name, task->name);
            *idx = *idx + 1;
            return 0;
        }
        n++;
    }
    /* 已经到达末尾了 */
    return -1;
}


/**
 * sys_getver - 获取系统版本
 * @buf: 缓冲区
 * @len: 缓冲区长度
 */
int sys_getver(char *buf, int len)
{
    if (len < strlen(OS_NAME) + strlen(OS_VERSION))
        return -1;

    strcpy(buf, OS_NAME);
    strcat(buf, OS_VERSION);
    return 0;
}

void dump_task(task_t *task)
{
    printk("----Task----\n");
    printk("name:%s pid:%d parent pid:%d state:%d\n", task->name, task->pid, task->parent_pid, task->state);
    //printk("vmm->vm_frame:%x priority:%d ticks:%d elapsed ticks:%d\n", task->vmm->page_storage, task->priority, task->ticks, task->elapsed_ticks);
    printk("exit code:%d stack magic:%d\n", task->exit_status, task->stack_magic);
}
static char *init_argv[2] = {"/sbin/init", 0};

/**
 * start_user - 开启用户进程
 * 
 * 在初始化的最后调用，当前任务演变成"idle"任务，等待随时调动
 */
void start_user()
{
    printk(KERN_DEBUG "[task]: start user process.\n");

    /* 加载init进程 */
    task_t *proc = start_process(init_argv[0], init_argv);
    if (proc == NULL)
        panic("kernel start process failed! please check initsrv!\n");

    /* 降级期间不允许产生中断 */
	unsigned long flags;
    save_intr(flags);
    /* 当前任务降级，这样，其它任务才能运行到 */
    task_kmain->priority = TASK_PRIO_IDLE;
    
    restore_intr(flags);
    /* 调度到更高优先级的任务允许 */
    schedule();
    /* 当没有任务运行时，就会继续执行这个地方 */
    
    /* 其它进程可能在终端关闭状态阻塞，那么切换回来后就需要打开中才行 */
    enable_intr();

    /* kmain线程 */
	while (1) {
		/* 进程默认处于阻塞状态，如果被唤醒就会执行后面的操作，
		直到再次被阻塞 */
        /* 执行cpu停机 */
		cpu_idle();
	};
}

void kthread_idle(void *arg)
{
    printk(KERN_DEBUG "[task]: idle start...\n");
    while (1) {
        cpu_idle();
    }
}
/**
 * init_tasks - 初始化多任务环境
 */
void init_tasks()
{
    init_schedule();

    next_pid = 0;
    /* 创建内核主线程 */
    create_kmain_thread();

    new_pid(); /* 跳过pid1，预留给INIT进程 */

    init_waitque(); /* 初始化用户态等待队列 */

    /* 有可能做测试阻塞main线程，那么就没有线程，
    在切换任务的时候就会出错，所以这里创建一个测试线程 */
    /*kthread_start("test", TASK_PRIO_RT, taskA, "NULL");
    kthread_start("test2", TASK_PRIO_RT, taskB, "NULL");
    kthread_start("test3", TASK_PRIO_RT, taskC, "NULL");*/
    //kthread_start("test4", 1, taskD, "NULL");
    sched_unit_t *su = sched_get_unit();
    su->idle = task_kmain;

    #if 0
    /* 创建idle线程 */
    kthread_start("idle", TASK_PRIO_IDLE, kthread_idle, NULL);
    #endif
    printk(KERN_INFO "[task]: init done\n");
}
