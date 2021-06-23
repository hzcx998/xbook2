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
#include <xbook/clock.h>
#include <xbook/vmm.h>
#include <xbook/safety.h>
#include <math.h>
#include <errno.h>
#include <xbook/semaphore.h>
#include <xbook/synclock.h>
#include <xbook/fifobuf.h>
#include <xbook/fifoio.h>
#include <xbook/rwlock.h>
#include <xbook/mutexqueue.h>
#include <xbook/process.h>
#include <xbook/exception.h>
#include <xbook/kernel.h>
#include <xbook/fd.h>
#include <xbook/fs.h>

static pid_t task_next_pid;
LIST_HEAD(task_global_list);
/* task init done flags, for early interrupt. */
volatile int task_init_done = 0;

/* 内核栈底部地址 */
char *kernel_stack_buttom;

task_t *kdeamon_task;

pid_t task_take_pid()
{
    return task_next_pid++;
}

void task_rollback_pid()
{
    --task_next_pid;
}

extern void signal_init(task_t *task);

void task_init(task_t *task, char *name, uint8_t prio_level)
{
    memset(task, 0, sizeof(task_t));
    strcpy(task->name, name);
    task->state = TASK_READY;
    spinlock_init(&task->lock);
    task->static_priority = sched_calc_base_priority(prio_level);
    task->priority = task->static_priority;
    //task->timeslice = TASK_TIMESLICE_BASE + 1;
    task->timeslice = TASK_TIMESLICE_BASE;
    task->ticks = task->timeslice;
    task->elapsed_ticks = 0;
    task->syscall_ticks = task->syscall_ticks_delta = 0;
    task->vmm = NULL;
    task->pid = task_take_pid();
    task->tgid = task->pid; /* 默认都是主线程，需要的时候修改 */
    task->pgid = -1;
    task->parent_pid = -1;
    task->exit_status = 0;
    task->trapframe = mem_alloc(PAGE_SIZE);
    assert(task->trapframe != NULL);
    // set kernel stack as the top of task mem struct
    task->kstack = (unsigned char *)(((unsigned long )task) + TASK_KERN_STACK_SIZE);
    task->flags = 0;
    fpu_init(&task->fpu, 0);
    task->errcode = 0;
    task->exit_hook = NULL;
    task->exit_hook_arg = NULL;
    task->stack_magic = TASK_STACK_MAGIC;
    task->fileman = NULL;
    exception_manager_init(&task->exception_manager);
    timer_init(&task->sleep_timer, 0, NULL, NULL);
    alarm_init(&task->alarm);
    signal_init(task);
    #ifdef CONFIG_PTHREAD
    task->pthread = NULL;
    #endif
    task->port_comm = NULL;
}

void task_free(task_t *task)
{
    list_del(&task->global_list);
    mem_free(task);
}

void task_add_to_global_list(task_t *task)
{
    assert(!list_find(&task->global_list, &task_global_list));
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

int task_is_child(pid_t pid, pid_t child_pid)
{
    task_t *child = task_find_by_pid(child_pid);
    if (!child)
        return 0;
    return (child->parent_pid == pid);
}

/**
 * kern_thread_start - 启动一个内核线程
 * @name: 线程的名字
 * @prio_level: 线程优先级
 * @func: 线程入口
 * @arg: 线程参数
 * 
 * @return: 成功返回任务的指针，失败返回NULL
 */
task_t *kern_thread_start(char *name, uint8_t prio_level, task_func_t *func, void *arg)
{
    
    task_t *task = (task_t *) mem_alloc(TASK_KERN_STACK_SIZE);
    if (!task)
        return NULL;
    task_init(task, name, prio_level);
    task->flags |= THREAD_FLAG_KERNEL;
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
    if (cur->pid == KEDEAMON_PID) {
        dbgprint("deamon thread can't exit!\n");
        interrupt_restore_state(flags);
        return;
    }
    cur->exit_status = status;
    task_do_cancel(cur);
    task_exit_hook(cur);
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
    assert(task != NULL);
    spin_lock(&task->lock);
    task->state = TASK_RUNNING;
    //dbgprint("[task] name=%s pid=%d active vmm\n", task->name, task->pid);
    vmm_active(task->vmm);
    spin_unlock(&task->lock);
}

void task_block(task_state_t state)
{
    unsigned long flags;
    interrupt_save_and_disable(flags);
    assert((state == TASK_BLOCKED) || 
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
        assert(!sched_queue_has_task(su, task));
        if (sched_queue_has_task(su, task)) {
            panic("task_unblock: task has already in ready list!\n");
        }
        task->state = TASK_READY;
        task->priority = sched_calc_new_priority(task, 1);
        sched_queue_add_head(su, task);
    }
    interrupt_restore_state(flags);
}

void task_yield()
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
    // su->idle = (task_t *) KERNEL_STATCK_BOTTOM;
    su->idle = (task_t *) kernel_stack_buttom;
    
    task_init(su->idle, "idle0", TASK_PRIO_LEVEL_REALTIME);
    /* 需要在后面操作文件，因此需要初始化文件描述符表 */
    if (fs_fd_init(su->idle) < 0) { 
        panic("init kmain fs fd failed!\n");
    }
    su->idle->state = TASK_RUNNING;
    task_add_to_global_list(su->idle);
    
    su->cur = su->idle;
}

pid_t task_get_pid(task_t *task)
{
    return task->tgid;
}

/* 
当调用者为进程时，tgid=pid
当调用者为线程时，tgid=master process pid
也就是说，线程返回的是主线程（进程）的pid
*/
pid_t sys_get_pid()
{
    return task_get_pid(task_current);
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

/**
 * 设置pgid时，进程只能为自己和子进程设置pgid
 */
int sys_set_pgid(pid_t pid, pid_t pgid)
{
    if (pid < 0 || pgid < -1)
        return -EINVAL;
    task_t *task = NULL;
    task_t *cur = task_current;
    
    if (!pid) { /* pid=0：get current task pgid */
        task = cur;
    } else {
        task = task_find_by_pid(pid);
        if (!task)
            return -ESRCH;
    }
    if (!pgid) {    /* 使用pid对应进程的pid */
        pgid = task->pid;
    }
    /* pid不是自己的子进程或者是自己就退出 */
    if (task->pid != cur->pid && !task_is_child(cur->pid, task->pid))
        return -EPERM;
    task->pgid = pgid;
    return 0;
}

pid_t sys_get_pgid(pid_t pid)
{
    if (pid < 0)
        return -EINVAL;
    task_t *task = NULL;
    if (!pid) { /* pid=0：get current task pgid */
        task = task_current;
    } else {
        task = task_find_by_pid(pid);
        if (!task)
            return -ESRCH;
    }
    return task->pgid;
}

void tasks_print()
{
    keprint("\n----Task----\n");
    task_t *task;
    list_for_each_owner(task, &task_global_list, global_list) {
        keprint("name %s pid %d ppid %d state %d\n", 
            task->name, task->pid, task->parent_pid,  task->state);
    }
}

int sys_tstate(tstate_t *ts, unsigned int *idx)
{
    if (!ts || !idx)
        return -EINVAL;
    unsigned int index;
    if (mem_copy_from_user(&index, idx, sizeof(unsigned int)) < 0)
        return -EINVAL;
    task_t *task;
    tstate_t tmp_ts;
    int n = 0;
    list_for_each_owner (task, &task_global_list, global_list) {
        if (n == index) {
            tmp_ts.ts_pid = task->pid;
            tmp_ts.ts_ppid = task->parent_pid;
            tmp_ts.ts_pgid = task->pgid;
            tmp_ts.ts_tgid = task->tgid;
            tmp_ts.ts_state = task->state;
            tmp_ts.ts_priority = task->priority;
            tmp_ts.ts_timeslice = task->timeslice;
            tmp_ts.ts_runticks = task->elapsed_ticks;
            memset(tmp_ts.ts_name, 0, PROC_NAME_LEN);
            strcpy(tmp_ts.ts_name, task->name);
            ++index;
            if (mem_copy_to_user(ts, &tmp_ts, sizeof(tstate_t)) < 0)
                return -EINVAL;
            if (mem_copy_to_user(idx, &index, sizeof(unsigned int)) < 0)
                return -EINVAL;
            return 0;
        }
        n++;
    }
    return -ESRCH;
}

int task_set_cwd(task_t *task, const char *path)
{
    if (!task || !path)
        return -EINVAL;
    int len = strlen(path);
    memset(task->fileman->cwd, 0, MAX_PATH);
    memcpy(task->fileman->cwd, path, min(len, MAX_PATH));
    return 0;
}

char *task_get_cwd()
{
    return task_current->fileman->cwd;
}

int sys_getver(char *buf, int len)
{
    if (!buf || !len)
        return -EINVAL;
    char tbuf[32] = {0};
    strcpy(tbuf, KERNEL_NAME);
    strcat(tbuf, "-");
    strcat(tbuf, KERNEL_VERSION);
    if (mem_copy_to_user(buf, tbuf, min(len, strlen(tbuf))) < 0)
        return -EFAULT;
    return 0;
}

unsigned long sys_unid(int id)
{
    unsigned long _id;
    /* id(0-7) pid(8-15) systicks(16-31) */
    _id = (id & 0xff) + ((task_current->pid & 0xff) << 8) + ((systicks & 0xffff) << 16);
    return _id;
}

void task_dump(task_t *task)
{
    keprint("----Task----\n");
    keprint("name:%s pid:%d parent pid:%d state:%d\n", task->name, task->pid, task->parent_pid, task->state);
    keprint("exit code:%d stack magic:%d\n", task->exit_status, task->stack_magic);
}

void kern_do_idle(void *arg)
{
    /* 等待子进程结束，并释放子进程资源。 */
    while (1) {
        int status = 0;
        int _pid;
        _pid = kewaitpid(-1, &status, 0);    /* wait any child exit */
        if (_pid > 0) {
            infoprint("idle: process[%d] exit with status %d.\n", _pid, status);
        }
    }
}



/* 在内核中打开标准输入输出 */
//#define STDIO_INKERN    
#define STDIO_DEVICE    "/dev/con0"

#ifdef CONFIG_TEST_MACHINE

// #define TEST_MACHINE_DEBUG
#define TEST_PROGRAM_PATH   ""
#define TEST_MACHINE_PREFIX "[test machine] "

static void do_one_test(char *argv[])
{
    /* 启动应用程序 */
    task_t *proc = process_create(argv, NULL, PROC_CREATE_INKERN);
    if (proc == NULL)
        panic(TEST_MACHINE_PREFIX"kernel start process failed! please check initsrv!\n");
    #ifdef TEST_MACHINE_DEBUG
    dbgprintln(TEST_MACHINE_PREFIX"create process %d ok", proc->pid);
    #endif
    /* 等待子进程结束 */
    int status = 0;
    int _pid;
    _pid = kewaitpid(-1, &status, 0);    /* wait any child exit */
    if (_pid > 0) {
        #ifdef TEST_MACHINE_DEBUG
        infoprint(TEST_MACHINE_PREFIX"process[%d] exit with status %d.\n", _pid, status);
        #endif
    }
}

// #define TEST_MACHINE_LOAD_FILE

// #define TEST_MACHINE_SINGLE_TEST
#if 1
#define TEST_MACHINE_SINGLE_TEST_NAME   TEST_PROGRAM_PATH"uname"
#else
#define TEST_MACHINE_SINGLE_TEST_NAME   "/bin/utests"
#endif

#include <dirent.h>

#ifdef TEST_MACHINE_LOAD_FILE
/**
 * 加载文件式执行
*/
static void test_machine_load_files()
{
    dirent_t de;
    int dir = fsif.opendir(TEST_PROGRAM_PATH);
    if (dir >= 0) {
        while (1) {
            if (fsif.readdir(dir, &de) < 0)
                break;
            if (de.d_attr & DE_DIR) {
                #ifdef TEST_MACHINE_DEBUG
                dbgprintln(TEST_MACHINE_PREFIX"dir %s/%s\n", TEST_PROGRAM_PATH, de.d_name);
                #endif
            } else {
                char path[128] = {0};
                strcpy(path, TEST_PROGRAM_PATH);
                strcat(path, "/");
                strcat(path, de.d_name);
                #ifdef TEST_MACHINE_DEBUG
                dbgprintln(TEST_MACHINE_PREFIX"file: %s", path);
                #endif
                char *test_argv[MAX_TASK_STACK_ARG_NR] = {0,};
                memset(test_argv, 0, sizeof(test_argv));
                /* 如果需要传入特殊参数，需要在此进行特殊处理 */
                test_argv[0] = path;
                if (!strcmp(de.d_name, "mount") || !strcmp(de.d_name, "umount")) {
                    test_argv[1] = "/dev/sda";
                    test_argv[2] = "/mnt";
                    test_argv[3] = 0;
                } else {
                    test_argv[1] = 0;
                }
                do_one_test(test_argv);
            }
        }
        fsif.closedir(dir);
    }
}
#else
static char *bin_name_talbe[] = {
    "brk",
    "chdir",
    "clone",
    "close",
    "dup2",
    "dup",
    "execve",
    "exit",
    "fork",
    "fstat",
    "getcwd",
    "getdents",
    "getpid",
    "getppid",
    "gettimeofday",
    "mkdir_",
    "mmap",
    "mount",
    "munmap",
    "openat",
    "open",
    "pipe",
    "read",
    "sleep",
    "times",
    "umount",
    "uname",
    "unlink",
    "wait",
    "waitpid",
    "write",
    "yield",
    NULL
};

/**
 * 通过加载名字方式执行
 */
static void test_machine_load_names()
{
    int i = 0;
    char *name = bin_name_talbe[i];
    while (name != NULL) {
        char path[128] = {0};
        strcpy(path, TEST_PROGRAM_PATH);
        strcat(path, "/");
        strcat(path, name);
        #ifdef TEST_MACHINE_DEBUG
        keprint(TEST_MACHINE_PREFIX"file: %s", path);
        #endif
        char *test_argv[MAX_TASK_STACK_ARG_NR] = {0,};
        memset(test_argv, 0, sizeof(test_argv));
        /* 如果需要传入特殊参数，需要在此进行特殊处理 */
        test_argv[0] = path;
        #if 0
        if (!strcmp(name, "mount") || !strcmp(name, "umount")) {
            test_argv[1] = "/dev/sda";
            test_argv[2] = "/mnt";
            test_argv[3] = 0;
        } else {
            test_argv[1] = 0;
        }
        #else
        test_argv[1] = 0;
        #endif
        do_one_test(test_argv);
        i++;
        name = bin_name_talbe[i];
    }
}
#endif
/* 测试机线程 */
void test_machine_thread(void *arg)
{
    dbgprintln(TEST_MACHINE_PREFIX"start...");
    /* 打开标准输入输出 */
    int fd = __sys_open(STDIO_DEVICE, 0);
    if (fd < 0)
        panic(TEST_MACHINE_PREFIX"start user: open stdin failed!");
    #ifdef TEST_MACHINE_DEBUG
    dbgprintln(TEST_MACHINE_PREFIX"start user: stdin fd %d", fd);
    #endif
    fd = __sys_open(STDIO_DEVICE, 0);
    if (fd < 0)
        panic(TEST_MACHINE_PREFIX"start user: open stdin failed!");
    #ifdef TEST_MACHINE_DEBUG
    dbgprintln(TEST_MACHINE_PREFIX"start user: stdout fd %d", fd);
    #endif
    fd = __sys_open(STDIO_DEVICE, 0);
    if (fd < 0)
        panic(TEST_MACHINE_PREFIX"start user: open stdin failed!");
    
    #ifdef TEST_MACHINE_DEBUG
    dbgprintln(TEST_MACHINE_PREFIX"start user: stderr fd %d", fd);
    #endif
    #ifdef TEST_MACHINE_SINGLE_TEST
    char *test_argv[MAX_TASK_STACK_ARG_NR] = {0,};
    test_argv[0] = TEST_MACHINE_SINGLE_TEST_NAME;
    do_one_test(test_argv);
    #else
    #ifdef TEST_MACHINE_LOAD_FILE
    test_machine_load_files();
    #else
    test_machine_load_names();
    #endif
    #endif
    #ifdef TEST_MACHINE_DEBUG
    dbgprintln(TEST_MACHINE_PREFIX"test all programs done.");
    #endif
    while (1) {
        cpu_idle();
        schedule();
    }
}
#else
#if defined(CONFIG_NEWSYSCALL)
#define USER_PROCESS_PATH  "/sbin/init"
#else
#define USER_PROCESS_PATH  "/sbin/init"
#endif
static char *init_argv[2] = {USER_PROCESS_PATH, 0};
#endif
/**
 * 在初始化的最后调用，当前任务演变成"idle"任务，等待随时调动
 */
void task_start_user()
{
    keprint(PRINT_DEBUG "[task]: start user process.\n");
    #ifndef CONFIG_TEST_MACHINE
    #ifdef STDIO_INKERN    
    int fd = __sys_open(STDIO_DEVICE, 0);
    if (fd < 0)
        panic("[task] start user: open stdin failed!");
    dbgprintln("[task] start user: stdin fd %d", fd);
    fd = __sys_open(STDIO_DEVICE, 0);
    if (fd < 0)
        panic("[task] start user: open stdin failed!");
    dbgprintln("[task] start user: stdout fd %d", fd);
    fd = __sys_open(STDIO_DEVICE, 0);
    if (fd < 0)
        panic("[task] start user: open stdin failed!");
    dbgprintln("[task] start user: stderr fd %d", fd);
    #endif
    task_t *proc = process_create(init_argv, NULL, PROC_CREATE_INKERN);
    if (proc == NULL)
        panic("kernel start process failed! please check initsrv!\n");
    /*keprintln("fisrt process pid=%d ppid=%d pgid=%d tgid=%d", 
        proc->pid, proc->parent_pid, proc->pgid, proc->tgid);
    */
    #else
    /* 启动测试机线程 */
    task_t *test_thread = kern_thread_start("test_machine", TASK_PRIO_LEVEL_LOW, test_machine_thread, NULL);
    assert(test_thread != NULL);
    if (test_thread == NULL)
        panic("start test machine failed!");
    #endif
    sched_unit_t *su = sched_get_cur_unit();
	unsigned long flags;
    interrupt_save_and_disable(flags);
    su->idle->static_priority = su->idle->priority = TASK_PRIORITY_LOW;
    interrupt_restore_state(flags);
    schedule();
    interrupt_enable();
    kern_do_idle(NULL);
}

/**
 * kdeamon线程是一个守护线程，和idle具有相同的优先级，
 * 当idle在进行初始化过程中需要调度出来，sleep或者yield时，
 * 就需要有其它线程来被调度，此时deamon的作用就出来了。
 * 如果没有kdeamon，那么内核可能只有一个线程，进行调度出去时，
 * 就会产生严重的错误！
 */
void kthread_deamon(void *arg)
{
    // infoprint("deamon thread start...");
    while (1) {
        #if 0
        cpu_idle();
        schedule();
        #else
        task_yield();
        #endif
    }
}

void tasks_init()
{
    task_next_pid = 0;
    sched_unit_t *su = sched_get_cur_unit();
    task_init_boot_idle(su);
    kdeamon_task = kern_thread_start("kdeamon", TASK_PRIO_LEVEL_LOW, kthread_deamon, NULL);
    assert(kdeamon_task != NULL);
    task_init_done = 1;
    keprint(PRINT_INFO "[task] init done");
}
