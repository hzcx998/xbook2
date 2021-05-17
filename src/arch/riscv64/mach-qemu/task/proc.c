#include <stddef.h>
#include <arch/proc.h>
#include <arch/riscv.h>
#include <arch/page.h>
#include <xbook/debug.h>
#include <xbook/list.h>
#include <arch/schedule.h>
#include <assert.h>
#include <string.h>
#include <xbook/memcache.h>

static pid_t task_next_pid;
LIST_HEAD(task_global_list);
/* proc init done flags, for early interrupt. */
volatile int task_init_done = 0;

extern char boot_stack[];   // 内核栈底

void reg_info(void) {
    keprint("register info: {\n");
    keprint("sstatus: %p\n", r_sstatus());
    keprint("sip: %p\n", r_sip());
    keprint("sie: %p\n", r_sie());
    keprint("sepc: %p\n", r_sepc());
    keprint("stvec: %p\n", r_stvec());
    keprint("satp: %p\n", r_satp());
    keprint("scause: %p\n", r_scause());
    keprint("stval: %p\n", r_stval());
    keprint("sp: %p\n", r_sp());
    keprint("tp: %p\n", r_tp());
    keprint("ra: %p\n", r_ra());
    keprint("}\n");
}
//#define THREAD_YIELD_ENABLE
#ifndef THREAD_YIELD_ENABLE
int thread_i = 0;
#endif

void thread_a(void *arg)
{
    keprint("thread a running. %s\n", (char *)arg);
    while (1)
    {
        #ifdef THREAD_YIELD_ENABLE
        infoprint("[A] I am running...\n");
        proc_yield();
        #else
        thread_i++;
        if (thread_i % 0xffffff == 0) {
            infoprint("[A] I am running...\n");
            infoprint("[A] interrupt state %d\n", interrupt_enabled());
        }
        #endif
    }
}

void thread_b(void *arg)
{
    keprint("thread b running.%s\n", (char *)arg);
    while (1)
    {
        #ifdef THREAD_YIELD_ENABLE
        infoprint("[B] I am running...\n");
        proc_yield();
        #else
        thread_i++;
        if (thread_i % 0xffffff == 0)
            infoprint("[B] I am running...\n");
        #endif
    }
}

void proc_yield()
{
    unsigned long flags;
    interrupt_save_and_disable(flags);
    task_current->state = TASK_READY;
    schedule();
    interrupt_restore_state(flags);
}

static void kernel_thread_entry()
{
    proc_t *cur = task_current;
    interrupt_enable();  /* 在启动前需要打开中断，避免启动后不能产生时钟中断调度 */
    cur->kthread_entry(cur->kthread_arg);
    // 如何函数返回了，那么就需要调用线程退出
    #if 1
    panic("kthread return!");
    #else
    kern_thread_exit(0);
    #endif
}

void proc_stack_build(proc_t *proc, task_func_t *function, void *arg)
{
    // Set up new context to start executing at forkret,
    // which returns to user space.
    memset(&proc->context, 0, sizeof(proc->context));
    proc->context.ra = (uint64_t)kernel_thread_entry;      // 函数执行入口
    proc->context.sp = (uint64_t)(proc->kstack + PAGE_SIZE); 
    proc->kthread_entry = function;
    proc->kthread_arg = arg;
    keprint("proc=%s sp=%p\n", proc->name, proc->context.sp);
}

void proc_init(proc_t *proc, char *name, uint8_t prio_level)
{
    memset(proc, 0, sizeof(proc_t));
    strcpy(proc->name, name);
    proc->state = TASK_READY;
    spinlock_init(&proc->lock);
    proc->static_priority = sched_calc_base_priority(prio_level);
    proc->priority = proc->static_priority;
    proc->timeslice = TASK_TIMESLICE_BASE + 1;
    proc->ticks = proc->timeslice;
    proc->elapsed_ticks = 0;
    proc->pid = task_take_pid();
    // set kernel stack as the top of proc mem struct
    proc->kstack = (unsigned char *)(((unsigned long )proc) + TASK_KERN_STACK_SIZE);
    proc->flags = 0;
    proc->stack_magic = TASK_STACK_MAGIC;
}

void proc_add_to_global_list(proc_t *proc)
{
    assert(!list_find(&proc->global_list, &task_global_list));
    list_add_tail(&proc->global_list, &task_global_list);
}

// Look in the process table for an UNUSED proc.
// If found, initialize state required to run in the kernel,
// and return with p->lock held.
// If there are no free procs, or a memory allocation fails, return 0.
proc_t *kthread_create(char *name, uint8_t prio_level, task_func_t func, void *arg)
{
    proc_t *proc = (proc_t *) mem_alloc(TASK_KERN_STACK_SIZE);
    if (!proc)
        return NULL;
    proc_init(proc, name, prio_level);
    proc->flags |= THREAD_FLAG_KERNEL;
    proc_stack_build(proc, func, arg);
    unsigned long flags;
    interrupt_save_and_disable(flags);
    proc_add_to_global_list(proc);
    sched_unit_t *su = sched_get_cur_unit();
    sched_queue_add_tail(su, proc);
    interrupt_restore_state(flags);
    return proc;
}

/**
 * 内核主线程就是从boot到现在的执行流。到最后会演变成idle
 * 在这里，我们需要给与它一个身份，他才可以参与多线程调度
 */
static void task_init_boot_idle(sched_unit_t *su)
{
    dbgprintln("boot stack top:%p", boot_stack);
    su->idle = (proc_t *) boot_stack;
    proc_init(su->idle, "idle0", TASK_PRIO_LEVEL_REALTIME);
    #if 0
    /* 需要在后面操作文件，因此需要初始化文件描述符表 */
    if (fs_fd_init(su->idle) < 0) { 
        panic("init kmain fs fd failed!\n");
    }
    #endif
    su->idle->state = TASK_RUNNING;
    proc_add_to_global_list(su->idle);
    
    su->cur = su->idle;
}

pid_t task_take_pid()
{
    return task_next_pid++;
}

void kern_do_idle(void *arg)
{
    while (1) {
        cpu_idle();
        schedule();
    }
}

/**
 * 在初始化的最后调用，当前任务演变成"idle"任务，等待随时调动
 */
void task_start_other()
{
    keprint(PRINT_DEBUG "[task]: start other thread.\n");
    sched_unit_t *su = sched_get_cur_unit();
	unsigned long flags;
    interrupt_save_and_disable(flags);
    su->idle->static_priority = su->idle->priority = TASK_PRIORITY_LOW;
    interrupt_restore_state(flags);
    schedule();
    interrupt_enable();
    kern_do_idle(NULL);
}

// initialize the proc table at boot time.
void procinit(void)
{
    task_next_pid = 0;
    sched_unit_t *su = sched_get_cur_unit();
    task_init_boot_idle(su);
    task_take_pid(); /* 跳过pid1，预留给INIT进程 */
    task_init_done = 1;
    infoprint("[proc] init done.\n");

    proc_t *p = kthread_create("testA", TASK_PRIORITY_HIGH, thread_a, "hello, A!");
    if (!p)
        infoprint("create testA bad.\n");

    p = kthread_create("testB", TASK_PRIORITY_HIGH, thread_b, "hello, B!");
    if (!p)
        infoprint("create testB bad.\n");

    p = kthread_create("testC", TASK_PRIORITY_HIGH, thread_b, "hello, C!");
    if (!p)
        infoprint("create testC bad.\n");

}
