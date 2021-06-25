#include <xbook/signal.h>
#include <errno.h>
#include <xbook/debug.h>
#include <xbook/task.h>
#include <xbook/syscall.h>
#include <xbook/schedule.h>
#include <xbook/process.h>
#include <xbook/safety.h>
#include <arch/interrupt.h>
#include <string.h>

// #define _DEBUG_SIGNAL

static void set_signal_action(signal_t *signal, int signo, struct sigaction *sa)
{
    /* 信号从1开始，所以要-1 */
    signal->action[signo - 1].sa_flags = sa->sa_flags;
    signal->action[signo - 1].sa_handler = sa->sa_handler;
    sigcopyset(&signal->action[signo - 1].sa_mask, &sa->sa_mask);
    signal->action[signo - 1].sa_restorer = sa->sa_restorer;
}

static void get_signal_action(signal_t *signal, int signo, struct sigaction *sa)
{
    /* 信号从1开始，所以要-1 */
    sa->sa_flags = signal->action[signo - 1].sa_flags;
    sa->sa_handler = signal->action[signo - 1].sa_handler;
    sigcopyset(&sa->sa_mask, &signal->action[signo - 1].sa_mask);
    sa->sa_restorer = signal->action[signo - 1].sa_restorer;
}

/**
 * calc_signal_left - 计算是否还有信号需要处理
 */
static void calc_signal_left(task_t *task)
{
    if (sigisemptyset(&task->signal_pending))
        task->signal_left = 0;
    else 
        task->signal_left = 1;
}

static int del_signal_from_task(int signo, sigset_t *set)
{
    if (!sigismember(set, signo)) {
        return 0;
    }

    sigdelset(set, signo);

    /* 删除信号消息 */
    
    return 1;
}

/**
 * handle_stop_signal - 处理停止的信号
 * @signo: 信号
 * @task: 信号对应的任务
 * 
 * 在发送信号的时候，对一些信号的后续信号处理。
 * 某些信号的后续信号是不可以屏蔽的，在这里删除那些屏蔽
 * 为后续信号扫除障碍
 * 
 */
static void handle_stop_signal(int signo, task_t *task)
{
    switch (signo)
    {
    /* 如果是SIGKILL和SIGCONT，那么后续可能是SIGSTOP，SIGTSTP，SIGTTOU和SIGTTIN */
    case SIGKILL:
    /* 信号是CONT的时候，就会唤醒已经停止的进程，其实CONT的功能就在这里实现了，所以后面会忽略掉 */
    case SIGCONT:  
        /* 当发送的信号是必须要进程运行的条件时，需要唤醒它 */
        if (task->state == TASK_STOPPED) {
            //dbgprint(">>>send kill or cont to a stopped task\n");
            task_wakeup(task);
        }
        
        /* 退出状态置0 */
        task->exit_status = 0;

        /* 清除一些信号信号屏蔽 */
        del_signal_from_task(SIGSTOP, &task->signal_blocked);
        del_signal_from_task(SIGTSTP, &task->signal_blocked);
        del_signal_from_task(SIGTTOU, &task->signal_blocked);
        del_signal_from_task(SIGTTIN, &task->signal_blocked);
        
        break;
    /* 如果是SIGSTOP，SIGTSTP，SIGTTOU和SIGTTIN，那么后续可能是SIGCONT */
    case SIGSTOP:
    case SIGTSTP:
    case SIGTTOU:
    case SIGTTIN:
        /* 如果又重新停止一次，就需要把SIGCONT屏蔽清除 */
        del_signal_from_task(SIGCONT, &task->signal_blocked);
        break;
    default:
        break;
    }
}


/**
 * ignore_signal - 忽略信号
 * @signo: 信号
 * @task: 信号对应的任务
 * 
 * 当信号是忽略处理的时候，并且也没有加以屏蔽，那就忽略它。
 * 发送时尝试忽略一些信号，成功就返回1，不是就返回0
 */
static int ignore_signal(int signo, task_t *task)
{
    /* 信号为0，就忽略之 */
    if (!signo)
        return 1;

    /* 如果已经屏蔽了，那么，就不会被忽略掉，尝试投递 */
    if (sigismember(&task->signal_blocked, signo)) {
        //dbgprint(">>>a blocked signal.\n");
        return 0;
    }
        
    unsigned long sa_handler = (unsigned long)task->signals.action[signo - 1].sa_handler;
    
    /* 处理函数是用户自定义，不会被忽略掉 */
    if (sa_handler > 1)
        return 0;

    //dbgprint(">> sa_handler:%x\n", sa_handler);
    /* 如果处理函数是SIG_IGN，也就是一个忽略信号 */
    if (sa_handler == 1)
        return (signo != SIGCHLD);  /* 如果信号不是子进程那么就忽略，是就不能忽略 */

    /* 默认信号处理函数，handler是SIG_DFL时 */
    switch (signo)
    {
    /* 忽略掉这些信号 */
    case SIGCONT:
    case SIGWINCH:
    case SIGCHLD:
    case SIGURG:
        return 1;
    /* 不能忽略的信号 */
    case SIGTSTP:
    case SIGTTIN:
    case SIGTTOU:
        return 0;
    default:
        return 0;
    }
}

/**
 * deliver_signal - 信号投递
 * @signo: 信号
 * @sender: 发送者
 * @task: 任务
 * 
 * 成功返回0，失败返回-1
 */
static int deliver_signal(int signo, pid_t sender, task_t *task)
{
    /* 如果信号发送者出问题就失败 */
    if (sender < 0) {
        return -1;
    }
#ifdef _DEBUG_SIGNAL
    dbgprint("deliver signal %d from %d to %d\n", signo, task->pid, sender);
#endif
    /* 填写信号信息 */
    task->signals.sender[signo - 1] = sender;

    /* 设置对应的信号 */
    sigaddset(&task->signal_pending, signo);

    /* 如果投递后，该信号没有被屏蔽，那么就尝试唤醒该进程（如果没有睡眠就不管），
    保证它能够早点处理信号，
    如果说被屏蔽了，那么就不能唤醒它
    */
    if (!sigismember(&task->signal_blocked, signo)) {
        //dbgprint("not a blocked signal.\n");

        task->signal_left = 1;       /* 有信号剩余 */
        
        /* 如果是阻塞中，就唤醒它 */
        if (task->state == TASK_BLOCKED || task->state == TASK_WAITING || 
            task->state == TASK_STOPPED) {
            #ifdef _DEBUG_SIGNAL
            dbgprint("[deliver_signal] task %s is blocked, wake up it.", task->name);
            #endif
            
            
            /* 如果投递给一个进程，其处于休眠状态，就要将它唤醒，并且要取消它的休眠定时器 */
            task_do_cancel(task);

            task_unblock(task);
            /* 如果在投递过程中，任务处于休眠中，那么就要用休眠计时器去唤醒它 */
        }
    } else {
        //dbgprint("a blocked signal.\n");
    }

    /* 如果是投递了一个阻塞的信号，那么就没必要标明有信号剩余，不用处理 */

    return 0;
}

static void build_signal_frame(int signo, struct sigaction *sa, trap_frame_t *frame)
{
    /* 获取信号栈框，在用户的sp栈下面 */
    signal_frame_t *signal_frame = (signal_frame_t *)((frame->sp - sizeof(signal_frame_t)) & -8UL);
    task_t *cur = task_current; 
    #ifdef _DEBUG_SIGNAL
    dbgprint("trap frame %x, signal frame top %x,  base %x\n", 
    frame, frame->sp, signal_frame);
    #endif
    int err = 0;
    err |= mem_copy_to_user(&signal_frame->trap_frame, frame, sizeof(trap_frame_t));    /* 保存栈帧 */
    err |= mem_copy_to_user(&signal_frame->old_mask, &cur->signal_blocked, sizeof(sigset_t));    /* 保存信号屏蔽 */
    if (err < 0) 
        return;

    if (!(sa->sa_flags & SA_RESTORER) || !sa->sa_restorer) {
        errprint("[signal] no SA_RESTORER when build signal frame\n flags %x, restore %x\n", sa->sa_flags, sa->sa_restorer);
        return;
    }

    /* 修改阻塞标志 */
    sigcopyset(&cur->signal_blocked, &sa->sa_mask);

    if (sa->sa_flags & SA_NODEFER) {    /* 当前信号不会在执行期间阻塞 */
        sigdelset(&cur->signal_blocked, signo);
    }

    /* 设置返回地址 */
    frame->ra = (unsigned long)sa->sa_restorer;
    frame->epc = (unsigned long)sa->sa_handler;
	frame->sp = (unsigned long)signal_frame;
	frame->a0 = signo;                     /* a0: signal number */
}

/**
 * handle_signal - 处理信号
 * @frame: 中断栈框
 * @signo: 信号
 * 
 * 处理信号，分为默认信号，忽略信号，以及用户自定义信号
 * 
 * 成功返回0，失败返回-1
 */
static int handle_signal(trap_frame_t *frame, int signo)
{
    unsigned long iflags;
    interrupt_save_and_disable(iflags);
    /* 获取信号行为 */
    struct sigaction *sa = &task_current->signals.action[signo - 1];

    /* 处理自定义函数 */
    #ifdef _DEBUG_SIGNAL
    dbgprint("handle user function!\n");
    #endif

    /* 构建信号栈框，返回时就可以处理用户自定义函数 */
    build_signal_frame(signo, sa, frame);
    
    /* 执行完信号后需要把信号行为设置为默认的行为 */
    if (sa->sa_flags & SA_ONESHOT) {
        sa->sa_handler = SIG_DFL;
    }
    interrupt_restore_state(iflags);
    return 0;
}

/**
 * do_signal - 执行信号处理
 * frame: 内核栈 
 * 
 */
int do_signal(trap_frame_t *frame)
{
    task_t *cur = task_current;
    /* 如果没有信号，那么就直接返回 */
    if (!cur->signal_left)
        return -1;

    #if 0
    /* 
    判断是否是在用户空间进入了信号处理，如果是内核空间就返回
    信号属于对进程的控制，而非是内核线程，所以这里需要排除内核态的线程
     */
    if (frame->cs != USER_CODE_SEL) {
        return -1;
    }
    #endif
    
    /*
    获取信号并处理
    */
    int sig = 1;    // 从信号1开始检测

    struct sigaction *sa;
    /* 当还有剩余的就一直循环 */
    for (sig = 1; cur->signal_left && sig < _NSIG; sig++) {
        /* pending为1，blocked为0才可以处理该信号 */
        if (sigismember(&cur->signal_pending, sig) && !sigismember(&cur->signal_blocked, sig)) {
            #ifdef _DEBUG_SIGNAL
            dbgprint("task %d received signal %d from task %d.\n",
                cur->pid, sig, cur->signals.sender[sig - 1]);
            #endif
            
            /* 已经获取这个信号了，删除它 */
            sigdelset(&cur->signal_pending, sig);

            /* 删除发送者 */
            cur->signals.sender[sig - 1] = -1;
            calc_signal_left(cur);

            /* 指向信号对应的信号行为 */
            sa = &cur->signals.action[sig - 1];  

            /* 如果是忽略处理，那么就会把忽略掉 */
            if (sa->sa_handler == SIG_IGN) {
                /* 如果是不是子进程信号，那么就忽略掉 */
                if (sig != SIGCHLD)
                    continue;
                /* 是子进程，那么，当前进程作为父进程，应该做对应的处理
                ！！！注意！！！
                在我们的系统中，不打算用信号来处理子进程，所以该处理也将被忽略
                 */
                /* 子进程发送信号来了，进行等待。
                多所有子进程检测，如果没有子进程，也可以返回
                 */
                //dbgprint("parent %s recv SIGCHLD!\n");

                continue;
            }

            /* 如果是默认处理行为，就会以每个信号默认的方式进行处理 */
            if (sa->sa_handler == SIG_DFL) {

                /* 如果是Init进程收到信号，那就啥也不做。 */
                if (cur->pid == 0)
                    continue;
                
                switch (sig)
                {
                /* 这些信号的默认处理方式其实就是忽略处理 */
                case SIGCONT:
                case SIGCHLD:
                case SIGWINCH:
                case SIGURG:
                case SIGUSR1:
                case SIGUSR2:
                    /* 忽略处理：默认就是不处理 */
                    continue;
                
                case SIGTSTP:
                case SIGTTIN:
                case SIGTTOU:
                case SIGSTOP:
                    /* 进程暂停：改变状态为STOPPED */
                    //dbgprint("task %d stoped\n", cur->pid);
                    /* 停止运行处理 */
                    cur->state = TASK_STOPPED;
                    cur->exit_status = sig;
                    /* 调度出去 */
                    schedule();
                    continue;

                case SIGQUIT:
                case SIGILL:
                case SIGTRAP:
                case SIGABRT:
                case SIGFPE:
                case SIGSEGV:
                case SIGBUS:
                case SIGXCPU:
                case SIGXFSZ:
                    /* 进程流产：需要将进程转储，目前还不支持 */

                default:
                /* 进程终止：SIGUP, SIGINT, SIGKILL, SIGPIPE, SIGTERM, 
                SIGSTKFLT, SIGIO, SIGPOLL  */

                    /* 将当前信号添加到未决信号集，避免此时退出没有成功 */
                    sigaddset(&cur->signal_pending, sig);
                    /* 计算信号剩余 */
                    calc_signal_left(cur);
                    errprint("task %d exit because signal %d\n", cur->pid, sig);
                    /* 退出运行 */
                    sys_exit(sig);
                }
            }

            /* 处理信号 */
            handle_signal(frame, sig);
            
            /* 返回成功处理一个信号 */
            return 1;
        }
    }
    //dbgprint("do signal exit.\n");

    return 0;
}


/**
 * do_send_signal -发送信号
 * 
 */
int do_send_signal(pid_t pid, int signal, pid_t sender)
{
    /* 发送信号 */
    #ifdef _DEBUG_SIGNAL
    dbgprint("task %d sent signal %d to task %d.\n",
               sender, signal, pid);
    #endif
    /* 对参数进行检测 */
    if (IS_BAD_SIGNAL(signal)) {
        return -EINVAL;
    }
    task_t *task = task_find_by_pid(pid);
    
    /* 没找到要发送的进程，返回失败 */
    if (task == NULL) {
        return -ESRCH;
    }
    int ret = 0;

    /* 信号上锁并关闭中断 */
    unsigned long iflags;
    spin_lock_irqsave(&task->signal_mask_lock, iflags);

    /* 如果是停止相关信号，就扫清后续信号屏蔽 */
    handle_stop_signal(signal, task);
    //dbgprint("ignore.");
    /* 如果忽略成功，就退出 */
    if (ignore_signal(signal, task))
        goto out;
    //dbgprint("is member.");
    
    /* 如果已经处于未决，那就不投递 */
    if (sigismember(&task->signal_pending, signal))
        goto out;

    /* 进行信号投递 */
    //dbgprint("deliver.");
    
    ret = deliver_signal(signal, sender, task);

out:
    spin_unlock_irqrestore(&task->signal_mask_lock, iflags);
    /* 发送后，如果进程处于可中断状态，并且还有信号需要处理，那么就唤醒进程 */
    if ((task->state == TASK_BLOCKED || task->state == TASK_WAITING || 
        task->state == TASK_STOPPED) && !sigisemptyset(&task->signal_pending)) {
        //dbgprint("wakeup a blocked task %s after send a signal.\n", task->name);
        
        /* 唤醒任务 */
        task_wakeup(task);
        
        calc_signal_left(task);
    }
    
    return ret;
}

/**
 * do_send_signal -发送信号
 * 
 */
static int do_send_signal_group(pid_t gpid, int signal, pid_t sender)
{
    //dbgprint("do_send_signal_group\n");
    int retval = 0;
    if (gpid > 0) {
        task_t *task;
        foreach_task(task) {
            /* 发送给相同的组标的进程 */
            if (task->pgid == gpid) {
                int err = do_send_signal(task->pid, signal, task_current->pid);
                if (err < 0)      /* 如果有错误的，就记录成错误 */
                    retval = err;
            }
        }
    }
    return retval;
}

/**
 * force_signal - 强制发送一个信号
 * @signo: 信号
 * @task: 发送到的任务
 * 
 * 从内核中强制发送一个信号。
 * 通常是内核发生故障，例如浮点处理，管道处理出错之类的
 */
int force_signal(int signo, pid_t pid)
{
    task_t *task = task_find_by_pid(pid);
    /* 没找到要发送的进程，返回失败 */
    if (task == NULL) {
        return ESRCH;
    }

    /* 信号上锁并关闭中断 */
    unsigned long iflags;
    spin_lock_irqsave(&task->signal_mask_lock, iflags);
    /* 
    1.如果该信号的处理方法是忽略，那么就要变成默认处理方法，才会被处理
    2.由于要强制发送信号，那么该信号是不能被屏蔽的，于是要清除屏蔽
     */
    if (task->signals.action[signo - 1].sa_handler == SIG_IGN)
        task->signals.action[signo - 1].sa_handler = SIG_DFL;

    sigdelset(&task->signal_blocked, signo); /* 清除屏蔽阻塞 */ 

    /* 计算一下是否有信号需要处理 */
    calc_signal_left(task);
    
    spin_unlock_irqrestore(&task->signal_mask_lock, iflags);

    /* 正式发送信号过去 */
    return do_send_signal(task->pid, signo, task_current->pid);
}

int force_signal_self(int signo)
{
    return force_signal(signo, task_current->pid);
}

/**
 * send_branch - 发出信号给分支
 * @pid: 接收信号进程
 * @signal: 信号值
 * @sender: 信号发送者
 * 
 * pid > 0: 发送给pid进程
 * pid = 0：发送给当前进程的进程组
 * pid = -1：发送给有权的所有进程
 * pid < -1：发送给-pid进程组的进程
 * 
 * 根据pid选择发送方式
 */
static int send_branch(pid_t pid, int signal, pid_t sender)
{
    if (pid > 0) {  /* 发送给单个进程 */
        return do_send_signal(pid, signal, sender);
    } else if (pid == 0) { /* 发送给当前进程的进程组 */
        return do_send_signal_group(task_current->pgid, signal, sender);
    } else if (pid == -1) { /* 发送给有权利发送的所有进程 */
        int retval = 0, count = 0, err;
        task_t *task;
        task_t *cur = task_current;
        
        foreach_task(task) {
            if (task->pid > 0 && task != cur) {
                err = do_send_signal(task->pid, signal, cur->pid);
                count++;
                if (err < 0)      /* 如果有错误的，就记录成错误 */
                    retval = err;
            }
        }
        return retval;
    } else {    /* pid < 0，发送给-pid组标识的进程 */
        //dbgprint("pid is %d\n", pid);
        return do_send_signal_group(-pid, signal, sender);
    }
}

/**
 * sys_kill - 通过系统调用发送信号
 * @pid: 接收信号进程
 * @signal: 信号值
 * 
 * 发送时，会根据pid的值判定是发送给单个任务还是一组任务
 */
int sys_kill(pid_t pid, int signal)
{
    return send_branch(pid, signal, task_current->pid);
}

/**
 * 只把信号发送到tid对应的任务。
 */
int sys_tkill(pid_t tid, int signal)
{
    return do_send_signal(tid, signal, task_current->pid);
}

/**
 * sys_signal - 设定信号的处理函数
 * @signal: 信号
 * @sa_handler: 处理函数
 * 
 */
int sys_signal(int signal, sighandler_t sa_handler)
{
    /* 检测是否符合范围，并且不能是SIGKILL和SIGSTOP，这两个信号不允许设置响应 */
    if (signal < 1 || signal >= _NSIG ||
        (sa_handler && (signal == SIGKILL || signal == SIGSTOP))) {
        return -EINVAL;
    }

    /* 设定信号处理函数 */
    struct sigaction sa;
    sa.sa_handler = sa_handler;
    sa.sa_flags = SA_ONESHOT;     /* signal设定的函数都只捕捉一次 */
    
    task_t *cur = task_current;

    /* 当需要修改信号行为的时候，就需要上锁 */
    spin_lock(&cur->signals.signal_lock);
    
    if (sa_handler) {
        /* 设置信号行为 */
        set_signal_action(&cur->signals, signal, &sa);

        /* 如果处理函数是忽略，或者是默认并且信号是SIGCONT,SIGCHLD,SIGWINCH，
        按照POSIX标准，需要把已经到达的信号丢弃 */
        if (sa.sa_handler == SIG_IGN || (sa.sa_handler == SIG_DFL && 
            (signal == SIGCONT || signal == SIGCHLD || signal == SIGWINCH))) {
            
            /* 如果需要修改信号未决和信号阻塞，就需要获取锁 */
            spin_lock(&cur->signal_mask_lock);

            /* 将信号从未决信号集中删除 */
            if (del_signal_from_task(signal, &cur->signal_pending)) {
                calc_signal_left(cur);
            }

            spin_unlock(&cur->signal_mask_lock);
        }
    }

    spin_unlock(&cur->signals.signal_lock);
    return 0;
}


/**
 * do_signal_action - 设定信号的处理行为
 * @signal: 信号
 * @act: 处理行为
 * @oldact: 旧的处理行为
 * 
 * 成功返回0，失败返回-1
 */
int do_signal_action(int signal, struct sigaction *act, struct sigaction *oldact)
{
    /* 检测是否符合范围，并且不能是SIGKILL和SIGSTOP，这两个信号不允许设置响应 */
    if (signal < 1 || signal >= _NSIG ||
        (act && (signal == SIGKILL || signal == SIGSTOP))) {
        return -1;
    }

    task_t *cur = task_current;

    struct sigaction sa;

    /* 当需要修改信号行为的时候，就需要上锁 */
    spin_lock(&cur->signals.signal_lock);
    
    /* 备份旧的行为 */
    if (oldact) {
        get_signal_action(&cur->signals, signal, &sa);
        /* 复制数据 */
        oldact->sa_flags = sa.sa_flags;
        oldact->sa_handler = sa.sa_handler;
        sigcopyset(&oldact->sa_mask, &sa.sa_mask);
        oldact->sa_restorer = sa.sa_restorer;
        //dbgprint("old act sa_handler %x\n", oldact->sa_handler);
    }

    if (act) {
        sa.sa_flags = act->sa_flags;
        sa.sa_handler = act->sa_handler;
        sigcopyset(&sa.sa_mask, &act->sa_mask);
        sa.sa_restorer = act->sa_restorer;

        //dbgprint("new act sa_handler %x\n", act->sa_handler);
        /* 设置信号行为 */
        set_signal_action(&cur->signals, signal, &sa);

        /* 如果处理函数是忽略，或者是默认并且信号是SIGCONT,SIGCHLD,SIGWINCH，
        按照POSIX标准，需要把已经到达的信号丢弃 */
        if (sa.sa_handler == SIG_IGN || (sa.sa_handler == SIG_DFL && 
            (signal == SIGCONT || signal == SIGCHLD || signal == SIGWINCH))) {
            
            /* 如果需要修改信号未决和信号阻塞，就需要获取锁 */
            spin_lock(&cur->signal_mask_lock);

            /* 将信号从未决信号集中删除 */
            if (del_signal_from_task(signal, &cur->signal_pending)) {
                calc_signal_left(cur);
            }

            spin_unlock(&cur->signal_mask_lock);
        }
    }

    spin_unlock(&cur->signals.signal_lock);
    return 0;
}

/**
 * do_signal_process_mask - 设置进程的信号阻塞集
 * @how: 怎么设置阻塞
 * @set: 阻塞集
 * @oldset: 旧阻塞集
 * 
 * 如果set不为空，就要设置新集，如果oldset不为空，就要把旧集复制过去
 * 
 * return: 成功返回0，失败返回-1
 */
int do_signal_process_mask(int how, sigset_t *set, sigset_t *oldset)
{
    task_t *cur = task_current;

    /* 如果需要修改信号未决和信号阻塞，就需要获取锁 */
    spin_lock(&cur->signal_mask_lock);

    /* 有旧集就输出 */
    if (oldset != NULL) {
        *oldset = cur->signal_blocked;
    }

    if (how == SIG_BLOCK) {     /* 把新集中为1的添加到阻塞中 */
        /* 有新集就输入 */
        if (set != NULL) {
            sigorset(&cur->signal_blocked, set);
            /* 不能阻塞SIGKILL和SIGSTOP */
            sigdelset(&cur->signal_blocked, SIGKILL);
            sigdelset(&cur->signal_blocked, SIGSTOP);
        }
    } else if (how == SIG_UNBLOCK) {    /* 把新集中为1的从阻塞中删除 */
        /* 有新集就输入 */
        if (set != NULL) {
            sigandset(&cur->signal_blocked, set);
        }
    } else if (how == SIG_SETMASK) {    /* 直接设置阻塞集 */
        /* 有新集就输入 */
        if (set != NULL) {
            sigcopyset(&cur->signal_blocked, set);

            /* 不能阻塞SIGKILL和SIGSTOP */
            sigdelset(&cur->signal_blocked, SIGKILL);
            sigdelset(&cur->signal_blocked, SIGSTOP);
        }
    } else {

        spin_unlock(&cur->signal_mask_lock);
        return -1;
    }

    spin_unlock(&cur->signal_mask_lock);
    return 0;
}

/**
 * 设置信号行为
 */
int sys_rt_sigaction(int sig,
        const struct sigaction *act,
		struct sigaction *oact,
		size_t sigactsize)
{
    if (sigactsize != sizeof(struct sigaction))
        return -EINVAL;
    struct sigaction _act, _oact;
    int err;
    if (act)
        err = mem_copy_from_user(&_act, (void *)act, sizeof(struct sigaction));
    
    if (err < 0)
        return err;
    
    err = do_signal_action(sig, act != NULL ? &_act: NULL, oact != NULL ? &_oact: NULL);
    if (err < 0)
        return err;
    if (oact)
        err = mem_copy_to_user(oact, &_oact, sizeof(struct sigaction));
    return err;
}        

int sys_rt_sigprocmask(int how, sigset_t *nset,
		sigset_t *oset, size_t sigsetsize)
{
    if (sigsetsize != sizeof(sigset_t))
        return -EINVAL;
    sigset_t _nset, _oset;
    int err;
    if (nset)
        err = mem_copy_from_user(&_nset, nset, sizeof(sigset_t));
    if (err < 0)
        return err;
    err = do_signal_process_mask(how, nset != NULL ? &_nset: NULL, oset != NULL ? &_oset: NULL);
    if (err < 0)
        return err;
    if (oset)
        err = mem_copy_to_user(oset, &_oset, sizeof(sigset_t));
    return err;
}

int do_rt_sigreturn()
{
    #ifdef _DEBUG_SIGNAL
    dbgprint("[signal] return from user\n");
    #endif
    trap_frame_t *frame = task_current->trapframe;
    signal_frame_t *signal_frame = (signal_frame_t *)(frame->sp);
    task_t *cur = task_current;
    #ifdef _DEBUG_SIGNAL
    dbgprint("[signal] frame %p signal frame %p\n", frame, signal_frame);
    #endif
    
    /* restore mask */
    if (mem_copy_from_user(&cur->signal_blocked, &signal_frame->old_mask, sizeof(sigset_t)) < 0)
        goto bad_return;
    /* restore frame */
    if (mem_copy_from_user(frame, &signal_frame->trap_frame, sizeof(trap_frame_t)) < 0)
        goto bad_return;
    return frame->a0;
bad_return:
    force_signal_self(SIGSEGV);
    errprint("[signal] signal return from user failed!");
    return 0;
}

int sys_rt_sigreturn()
{
    return do_rt_sigreturn();
}

void signal_init(task_t *task)
{
    task->signal_left = 0;
    task->signal_catched = 0;

    spinlock_init(&task->signals.signal_lock);
    int i;
    for (i = 0; i < _NSIG; i++) {
        task->signals.action[i].sa_handler = SIG_DFL;
        task->signals.action[i].sa_flags = 0;
        sigemptyset(&task->signals.action[i].sa_mask);
        task->signals.action[i].sa_restorer = NULL;
        task->signals.sender[i] = -1;
    }
    atomic_set(&task->signals.count, 0);
    
    /* 清空信号集 */
    sigemptyset(&task->signal_blocked);
    sigemptyset(&task->signal_pending);

    spinlock_init(&task->signal_mask_lock);
}
