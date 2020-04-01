#include <xbook/schedule.h>
#include <xbook/task.h>
#include <xbook/assert.h>
#include <xbook/debug.h>
#include <arch/interrupt.h>
#include <arch/task.h>

extern task_t *task_idle;

/* 优先级队列 */
priority_queue_t priority_queue[MAX_PRIORITY_NR];

/* 最高等级的队列 */
priority_queue_t *highest_prio_queue;

trap_frame_t *current_trap_frame;

/* 需要调度标志 */
int need_sched;

task_t *task_priority_queue_fetch_first()
{
    task_t *task;
    
    /* 当最高优先级的长度为0，就降低最高优先级到长度不为0的优先级，直到到达最低的优先级 */
    ADJUST_HIGHEST_PRIO(highest_prio_queue);
    /* get first task */
    task = list_first_owner(&highest_prio_queue->list, task_t, list);
    --highest_prio_queue->length;   /* sub a task */
    task->prio_queue = NULL;
    list_del_init(&task->list);
    /* 当最高优先级的长度为0，就降低最高优先级到长度不为0的优先级 */
    ADJUST_HIGHEST_PRIO(highest_prio_queue);
    return task;
}

task_t *get_next_task(task_t *task)
{
    //printk("cur %s-%d ->", task->name, task->pid);
    /* 1.如果是时间片到了就插入到就绪队列，准备下次调度，如果是其他状态就不插入就绪队列 */
    if (task->state == TASK_RUNNING) {
        /* 时间片到了，加入就绪队列 */
        task_priority_queue_add_tail(task);
        // 更新信息
        task->ticks = task->timeslice;
        task->state = TASK_READY;
    }
    /* 2.从就绪队列中获取一个任务 */
    /* 一定能够找到一个任务，因为最后的是idle任务 */
    task_t *next;
    next = task_priority_queue_fetch_first();
    return next;
}

void set_next_task(task_t *next)
{
    task_current = next;

    /* 如果不是阻塞时产生的临时栈，就指向默认中断栈，不然就指向临时栈。
    为什么需要这么做呢？
    因为时钟中断结束时，会把当前任务的默认栈当做中断栈退出。而这是切换任务的
    唯一方式，因此，如果我们想在非中断状态下实现任务切换，就需要模拟这个机制。
    如果我们模拟过程中使用了这个默认的栈，那么模拟结束后，返回的时候，会发现
    这个栈的内容以经被改变了，会出错。因此，引入了一个临时的栈，这样，就可以
    在中断执行过程中或者从用户态切换到内核态之中使用模拟中断执行退出机制，在
    中断中，或者是用户消息中，提前进行任务切换。不过，这个还是基于时钟中断机
    制，但是，他可以发生在用户消息之中。
    简单说，就是为了解决用户消息需要阻塞进程，而进场恢复后中断栈不一致的问题。
     */
    if (!task_current->flags) 
        current_trap_frame = (trap_frame_t *)task_current->kstack;
    else 
        current_trap_frame = task_current->block_frame;

    task_activate(task_current);
}

/**
 * Schedule - 任务调度
 * 
 * 把自己放入队列末尾。
 * 从优先级最高的队列中选取一个任务来执行。
 */
void schedule()
{
    task_t *cur = current_task;
    task_t *next = get_next_task(cur);
    set_next_task(next);
    need_sched = 1; /* 需要进行调度 */
    //printk(KERN_INFO "> switch from %d-%x to %d-%x addr=%x eip=%x\n", cur->pid, cur, next->pid, next, current_trap_frame, current_trap_frame->eip);
    //dump_trap_frame(current_trap_frame);
}

void launch_task()
{
    char *argv[3] = {"init", "arg2", 0};
    process_create("init", argv);
    
    /* 启动第一个任务 */
    set_next_task(task_priority_queue_fetch_first());
    printk(KERN_INFO "launch: name=%s, pid=%d ppid=%d\n", current_task->name,
        current_task->pid, current_task->parent_pid);
    switch_to_user(current_trap_frame);
}

#ifdef CONFIG_PREEMPT
/**
 * 抢占时机：
 * 1.创建一个新任务
 * 2.任务被唤醒，并且优先级更高
 */
void schedule_preempt(task_t *robber)
{
    /* close intr */
    /*unsigned long flags;
    save_intr(flags);*/
    
    task_t *cur = current_task;
    
    /* 自己不能抢占自己 */
    if (cur == robber)
        return;

    /* 当前任务一顶是在运行中，当前任务被其它任务抢占
    把当前任务放到自己的优先级队列的首部，保留原有时间片
     */
    cur->state = TASK_READY;
    task_priority_queue_add_tail(cur);
    
    /* 最高优先级指针已经在抢占前指向了抢占者的优先级，这里就不用调整了 */

    /* 抢占者需要从就绪队列中删除 */
    --highest_prio_queue->length;   /* sub a task */
    robber->prio_queue = NULL;
    list_del_init(&robber->list);

    /* 当最高优先级的长度为0，就降低最高优先级到长度不为0的优先级 */
    ADJUST_HIGHEST_PRIO(highest_prio_queue);
    
    
    /* 3.激活任务的内存环境 */
    task_activate(robber);
    task_current = robber;
    
    /* 4.切换到该任务运行 */
    switch_to(cur, robber);
    
    //restore_intr(flags);
    
}
#endif

void init_schedule()
{
    /* 初始化特权队列 */
    int i;
    for (i = 0; i < MAX_PRIORITY_NR; i++) {
        INIT_LIST_HEAD(&priority_queue[i].list);    
        priority_queue[i].length = 0; 
        priority_queue[i].priority = i;
    }
    /* 指向最高级的队列 */
    highest_prio_queue = &priority_queue[0];
    current_trap_frame = NULL;
    need_sched = 0;
}
