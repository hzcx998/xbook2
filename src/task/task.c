#include <arch/interrupt.h>
#include <arch/page.h>
#include <arch/cpu.h>
#include <arch/task.h>
#include <arch/phymem.h>
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
#include <xbook/mutexqueue.h>
#include <xbook/process.h>
#include <fsal/fsal.h>
#include <math.h>

static pid_t task_next_pid;
LIST_HEAD(task_global_list);
/* task init done flags, for early interrupt. */
volatile int task_init_done = 0;

pid_t task_alloc_pid()
{
    return task_next_pid++;
}

void task_init(task_t *task, char *name, int priority)
{
    memset(task, 0, sizeof(task_t));
    strcpy(task->name, name);
    task->state = TASK_READY;
    spinlock_init(&task->lock);
    if (priority < 0)
        priority = 0;
    if (priority > MAX_PRIORITY_NR - 1)
        priority = MAX_PRIORITY_NR - 1;
    task->static_priority = priority;
    task->priority = priority;
    task->timeslice = 3;
    task->ticks = task->timeslice;
    task->elapsed_ticks = 0;
    task->vmm = NULL;
    task->pid = task_alloc_pid();
    task->tgid = task->pid; /* 默认都是主线程，需要的时候修改 */
    task->parent_pid = -1;
    task->exit_status = 0;
    // set kernel stack as the top of task mem struct
    task->kstack = (unsigned char *)(((unsigned long )task) + TASK_KERN_STACK_SIZE);
    task->flags = 0;
    task->triggers = NULL;
    timer_init(&task->sleep_timer, 0, 0, NULL);
    alarm_init(&task->alarm);
    task->errno = 0;
    task->pthread = NULL;
    task->fileman = NULL;
    task->gmsgpool = NULL;
    task->stack_magic = TASK_STACK_MAGIC;
}

void task_free(task_t *task)
{
    list_del(&task->global_list);
    mem_free(task);
}

void task_add_to_global_list(task_t *task)
{
    ASSERT(!list_find(&task->global_list, &task_global_list));
    list_add_tail(&task->global_list, &task_global_list);
}

void task_set_timeslice(task_t *task, uint32_t timeslice)
{
    if (task) {
        if (timeslice < TASK_TIMESLICE_MIN)
            timeslice = TASK_TIMESLICE_MIN;
        if (timeslice > TASK_TIMESLICE_MAX)
            timeslice = TASK_TIMESLICE_MAX;
        spin_lock(&task->lock);
        task->timeslice = timeslice;
        spin_unlock(&task->lock);
        
    }
}

task_t *task_find_by_pid(pid_t pid)
{
    task_t *task;
    unsigned long flags;
    interrupt_save_and_disable(flags);
    list_for_each_owner(task, &task_global_list, global_list) {
        if (task->pid == pid) {
            interrupt_restore_state(flags);
            return task;
        }
    }
    interrupt_restore_state(flags);
    return NULL;
}

/**
 * kern_thread_start - 启动一个内核线程
 * @name: 线程的名字
 * @priority: 线程优先级
 * @func: 线程入口
 * @arg: 线程参数
 * 
 * @return: 成功返回任务的指针，失败返回NULL
 */
task_t *kern_thread_start(char *name, int priority, task_func_t *func, void *arg)
{
    task_t *task = (task_t *) mem_alloc(TASK_KERN_STACK_SIZE);
    if (!task)
        return NULL;
    task_init(task, name, priority);
    if (fs_fd_init(task) < 0) {
        mem_free(task);
        return NULL;
    }
    task_stack_build(task, func, arg);
    unsigned long flags;
    interrupt_save_and_disable(flags);
    task_add_to_global_list(task);
    sched_unit_t *su = sched_get_cur_unit();
    sched_queue_add_tail(su, task);
    interrupt_restore_state(flags);
    return task;
}

void kern_thread_exit(int status)
{
    unsigned long flags;
    interrupt_save_and_disable(flags);

    task_t *cur = task_current;
    cur->exit_status = status;
    task_do_cancel(cur);
    cur->parent_pid = USER_INIT_PROC_ID;
    task_t *parent = task_find_by_pid(cur->parent_pid); 
    if (parent) {
        if (parent->state == TASK_WAITING) {
            interrupt_restore_state(flags);
            task_unblock(parent);
            task_block(TASK_HANGING);
        } else {
            interrupt_restore_state(flags);
            task_block(TASK_ZOMBIE);
        }
    } else {
        interrupt_restore_state(flags);
        task_block(TASK_ZOMBIE); 
    }
}

void task_activate_when_sched(task_t *task)
{
    ASSERT(task != NULL);
    spin_lock(&task->lock);
    task->state = TASK_RUNNING;
    spin_unlock(&task->lock);
    vmm_active(task->vmm);
}

void task_block(task_state_t state)
{
    unsigned long flags;
    interrupt_save_and_disable(flags);
    ASSERT ((state == TASK_BLOCKED) || 
            (state == TASK_WAITING) || 
            (state == TASK_STOPPED) ||
            (state == TASK_HANGING) ||
            (state == TASK_ZOMBIE));
    task_t *current = task_current;
    current->state = state;    
    schedule();
    interrupt_restore_state(flags);
}

void task_unblock(task_t *task)
{
    unsigned long flags;
    interrupt_save_and_disable(flags);
    if (!((task->state == TASK_BLOCKED) || 
        (task->state == TASK_WAITING) ||
        (task->state == TASK_STOPPED))) {
        panic("task_unblock: task name=%s pid=%d state=%d\n", task->name, task->pid, task->state);
    }
    if (task->state != TASK_READY) {
        sched_unit_t *su = sched_get_cur_unit();
        ASSERT(!sched_queue_has_task(su, task));
        if (sched_queue_has_task(su, task)) {
            panic("task_unblock: task has already in ready list!\n");
        }
        task->state = TASK_READY;
        sched_queue_add_head(su, task);
    }
    interrupt_restore_state(flags);
}

void task_yeild()
{
    unsigned long flags;
    interrupt_save_and_disable(flags);
    task_current->state = TASK_READY;
    schedule();
    interrupt_restore_state(flags);
}

int task_count_children(task_t *parent)
{
    int children = 0;
    task_t *child;
    list_for_each_owner (child, &task_global_list, global_list) {
        if (child->parent_pid == parent->pid && TASK_IS_SINGAL_THREAD(child)) {
            children++;
        }
    }
    return children;
}

int task_do_cancel(task_t *task)
{
    timer_cancel(&task->sleep_timer);
    return 0;
}

/**
 * 内核主线程就是从boot到现在的执行流。到最后会演变成idle
 * 在这里，我们需要给与它一个身份，他才可以参与多线程调度
 */
static void task_init_boot_idle(sched_unit_t *su)
{
    su->idle = (task_t *) KERNEL_STATCK_BOTTOM;
    task_init(su->idle, "idle0", TASK_PRIO_BEST);
    /* 需要在后面操作文件，因此需要初始化文件描述符表 */
    if (fs_fd_init(su->idle) < 0) { 
        panic("init kmain fs fd failed!\n");
    }
    su->idle->state = TASK_RUNNING;
    task_add_to_global_list(su->idle);
    
    su->cur = su->idle;
}

/* 
当调用者为进程时，tgid=pid
当调用者为线程时，tgid=master process pid
也就是说，线程返回的是主线程（进程）的pid
*/
pid_t sys_get_pid()
{
    return task_current->tgid;
}

pid_t sys_get_ppid()
{
    return task_current->parent_pid;
}

/* 由于最小粒度是线程，所以，线程id=pid。 */
pid_t sys_get_tid()
{
    return task_current->pid;
}

void tasks_print()
{
    printk("\n----Task----\n");
    task_t *task;
    list_for_each_owner(task, &task_global_list, global_list) {
        printk("name %s pid %d ppid %d state %d\n", 
            task->name, task->pid, task->parent_pid,  task->state);
    }
}

int sys_tstate(tstate_t *ts, unsigned int *idx)
{
    if (ts == NULL)
        return -1;
    int n = 0;
    task_t *task;
    list_for_each_owner (task, &task_global_list, global_list) {
        if (n == *idx) {
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
    return -1;
}

int sys_getver(char *buf, int len)
{
    if (len < strlen(OS_NAME) + strlen(OS_VERSION))
        return -1;

    strcpy(buf, OS_NAME);
    strcat(buf, OS_VERSION);
    return 0;
}

void task_dump(task_t *task)
{
    printk("----Task----\n");
    printk("name:%s pid:%d parent pid:%d state:%d\n", task->name, task->pid, task->parent_pid, task->state);
    printk("exit code:%d stack magic:%d\n", task->exit_status, task->stack_magic);
}

void kern_do_idle(void *arg)
{
    while (1) {
        cpu_idle();
        schedule();
    }
}

#ifdef CONFIG_GRAPH
#define INIT_SBIN_PATH  "/sbin/initg"
#else
#define INIT_SBIN_PATH  "/sbin/init"
#endif

static char *init_argv[2] = {INIT_SBIN_PATH, 0};

/**
 * 在初始化的最后调用，当前任务演变成"idle"任务，等待随时调动
 */
void task_start_user()
{
    printk(KERN_DEBUG "[task]: start user process.\n");
    task_t *proc = user_process_start(init_argv[0], init_argv);
    if (proc == NULL)
        panic("kernel start process failed! please check initsrv!\n");
    sched_unit_t *su = sched_get_cur_unit();
    /* 降级期间不允许产生中断，降级后其它任务才有机会运行 */
	unsigned long flags;
    interrupt_save_and_disable(flags);
    su->idle->static_priority = su->idle->priority = TASK_PRIO_IDLE;
    interrupt_restore_state(flags);
    schedule();
    interrupt_enable();
    kern_do_idle(NULL);
}

void tasks_init()
{
    task_next_pid = 0;
    sched_unit_t *su = sched_get_cur_unit();
    task_init_boot_idle(su);
    task_alloc_pid(); /* 跳过pid1，预留给INIT进程 */
    task_init_done = 1;
    printk(KERN_INFO "[ok] tasks init.");
}
