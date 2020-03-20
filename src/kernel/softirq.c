#include <arch/interrupt.h>
#include <xbook/debug.h>
#include <xbook/softirq.h>
#include <xbook/memops.h>
#include <xbook/bitops.h>

/* 普通任务协助队列 */
task_assist_head_t task_assist_head;

/* 高级任务协助队列 */
task_assist_head_t high_task_assist_head;

/* 软中断表 */
static softirq_action_t softirq_table[NR_SOFTIRQS];

/* 软中断事件的标志 */
static unsigned long softirq_evens;

/**
 * get_softirq_evens - 获取软中断事件
 */
static unsigned long get_softirq_evens()
{
    return softirq_evens;
}

/**
 * set_softirq_evens - 设置软中断事件
 */
static void set_softirq_evens(unsigned long evens)
{
    softirq_evens = evens;
}

/**
 * build_softirq - 构建软中断
 * @softirq: 软中断号
 * @action: 软中断的行为
 * 
 * 构建后，软中断才可以激活，才可以找到软中断行为处理
 */
void build_softirq(unsigned long softirq, void (*action)(softirq_action_t *))
{
    /* 在范围内才进行设定 */
    if (0 <= softirq && softirq < NR_SOFTIRQS) {
        /* 把活动添加到软中断表中 */
        softirq_table[softirq].action = action;    
    }
}

/**
 * active_softirq - 激活软中断
 * @softirq: 软中断号
 * 
 * 激活后，每当有硬件中断产生，或者主动调用软中断处理的地方时，才会去执行软中断
 */
void active_softirq(unsigned long softirq)
{
    /* 在范围内才进行设定 */
    if (0 <= softirq && softirq < NR_SOFTIRQS) {
        /* 在对应位置修改软中断事件 */
        if (softirq_table[softirq].action)
            softirq_evens |= (1 << softirq);  
    }
}


/**
 * handle_softirq - 处理软中断
 */
static void handle_softirq()
{
    softirq_action_t *action;

    unsigned long evens;
    /* 再次处理计数 */
    int redoIrq = MAX_REDO_IRQ;

    /* 获取软中断事件 */
    evens = get_softirq_evens();

/*如果在处理软中断过程中又产生了中断，就会导致事件变化，如果
量比较大，就在这里多做几次处理*/
redo:
    /* 如果有事件，就会逐个运行 */
    if (evens) {
        /* 已经获取了，就置0 */
        set_softirq_evens(0);
        
        /* 如果有事件，就会逐个运行，由于事件运行可能会花费很多时间，
        所以开启中断，允许中断产生 */
        action = &softirq_table[0];
        
        /* 打开中断，允许中断产生 */
        enable_intr();

        /* 处理softirq事件 */
        do {
            /* 如果有软中断事件 */
            if (evens & 1)
                action->action(action);     /* 执行软中断事件 */
            
            /* 指向下一个行为描述 */
            action++;
            /* 指向下一个事件 */
            evens >>= 1;
        } while(evens);

        /* 关闭中断 */
        disable_intr();

        /* 检查是否还有事件没有处理，也就是说在处理事件过程中，又发生了中断 */
        evens = get_softirq_evens();
        
        /* 如果有事件，并且还可以继续尝试处理事件就继续运行 */
        if (evens && --redoIrq)
            goto redo;
    }   
    /* 已经处理完了，可以返回了 */
}

/**
 * do_softirq - 做软中断处理
 * 
 * 通过这个地方，将有机会去处理软中断事件
 */
void do_softirq()
{
    unsigned long evens;

    /* 如果当前硬件中断嵌套，或者有软中断执行，则立即返回。*/

    /* 关闭中断 */
    unsigned long flags;
    save_intr(flags);
    
    /* 获取事件 */
    evens = get_softirq_evens();

    /* 有事件就做软中断的处理 */
    if (evens) 
        handle_softirq();

    /* 恢复之前状态 */
    restore_intr(flags);
}

/**
 * high_task_assist_schedule - 高级任务协助调度
 * @assist: 任务协助
 * 
 * 对一个任务协助进行调度，它后面才可以运行
 */
void high_task_assist_schedule(task_assist_t *assist)
{
    /* 如果状态还没有调度，才能进行调度 */
    if (!test_and_set_bit(HIGHTASK_ASSIST_SOFTIRQ, &assist->status)) {
        unsigned long flags;
        save_intr(flags);

        /* 把任务协助插入到队列最前面 */
        assist->next = high_task_assist_head.head;
        high_task_assist_head.head = assist;

        /* 激活HIGHTTASKASSIST_SOFTIRQ */
        active_softirq(HIGHTASK_ASSIST_SOFTIRQ);

        restore_intr(flags);
    }
}

/**
 * high_task_assist_action - 高级任务协助行为处理 
 * @action: 行为
 */
static void high_task_assist_action(softirq_action_t *action)
{
    task_assist_t *list;

    /* 先关闭中断，不然修改链表可能和添加链表产生排斥 */
    disable_intr();
    /* 获取链表头指针，用于寻找每一个调度的任务协助 */
    list = high_task_assist_head.head;
    /* 把头置空，用于后面添加协助 */
    high_task_assist_head.head = NULL;
    enable_intr();

    /* 开始获取并处理协助 */
    while (list != NULL) {
        task_assist_t *assist = list;
        list = list->next;
        
        /* 协助处于打开状态(count == 0, enable) */
        if (!atomic_get(&assist->count)) {
            /* 设置状态为空，不是调度状态 */
            clear_bit(TASK_ASSIST_SCHED, &assist->status);

            /* 执行协助的处理函数 */
            assist->func(assist->data);

            /* 如果协助可以运行，那么运行后就不运行后面重新添加到队列中，
            待下次运行 */
            continue;
        }

        /* 如果写成是关闭状态，那么就重新加入到队列 */
        
        /* 修改链表数据时禁止中断 */
        disable_intr();
        /* 把任务协助插入到队列最前面 */
        assist->next = high_task_assist_head.head;
        high_task_assist_head.head = assist;

        /* 激活HIGHTASK_ASSIST_SOFTIRQ */
        active_softirq(HIGHTASK_ASSIST_SOFTIRQ);
        
        enable_intr();
        
    }
}

/**
 * task_assist_schedule - 普通任务协助调度
 * @assist: 任务协助
 * 
 * 对一个任务协助进行调度，它后面才可以运行
 */
void task_assist_schedule(task_assist_t *assist)
{
    /* 如果状态还没有调度，才能进行调度 */
    if (!test_and_set_bit(TASK_ASSIST_SOFTIRQ, &assist->status)) {
        unsigned long flags;
        save_intr(flags);

        /* 把任务协助插入到队列最前面 */
        assist->next = task_assist_head.head;
        task_assist_head.head = assist;

        /* 激活TASK_ASSIST_SOFTIRQ */
        active_softirq(TASK_ASSIST_SOFTIRQ);

        restore_intr(flags);
    }
}


/**
 * task_assist_action - 普通任务协助行为处理 
 * @action: 行为
 */
static void task_assist_action(softirq_action_t *action)
{
    task_assist_t *list;

    /* 先关闭中断，不然修改链表可能和添加链表产生排斥 */
    disable_intr();
    /* 获取链表头指针，用于寻找每一个调度的任务协助 */
    list = task_assist_head.head;
    /* 把头置空，用于后面添加协助 */
    task_assist_head.head = NULL;
    enable_intr();

    /* 开始获取并处理协助 */
    while (list != NULL) {
        task_assist_t *assist = list;
        list = list->next;
        
        /* 协助处于打开状态(count == 0, enable) */
        if (!atomic_get(&assist->count)) {
            /* 设置状态为空，不是调度状态 */
            clear_bit(TASK_ASSIST_SCHED, &assist->status);

            /* 执行协助的处理函数 */
            assist->func(assist->data);

            /* 如果协助可以运行，那么运行后就不运行后面重新添加到队列中，
            待下次运行 */
            continue;
        }

        /* 如果写成是关闭状态，那么就重新加入到队列 */
        
        /* 修改链表数据时禁止中断 */
        disable_intr();
        /* 把任务协助插入到队列最前面 */
        assist->next = task_assist_head.head;
        task_assist_head.head = assist;

        /* 激活TASK_ASSIST_SOFTIRQ */
        active_softirq(TASK_ASSIST_SOFTIRQ);
        
        enable_intr();
        
    }
}

/**
 * init_softirq - 初始化软中断
 */
void init_softirq()
{
    /* 初始化软中断事件 */
    softirq_evens = 0;

    /* 设置高级任务协助头为空 */
    high_task_assist_head.head = NULL;
    
    /* 设置普通任务协助头为空 */
    task_assist_head.head = NULL;
    
    /* 注册高级任务协助软中断 */
    build_softirq(HIGHTASK_ASSIST_SOFTIRQ, high_task_assist_action);

    /* 注册普通任务协助软中断 */
    build_softirq(TASK_ASSIST_SOFTIRQ, task_assist_action);
}
