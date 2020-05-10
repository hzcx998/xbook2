#ifndef _SYS_SYSCALL_H
#define _SYS_SYSCALL_H

enum syscall_num {
    SYS_EXIT,
    SYS_FORK,
    SYS_EXECR,
    SYS_EXECF,
    SYS_WAITPID,
    SYS_COEXIST,
    SYS_GETPID,
    SYS_GETPPID,
    SYS_TRIGGER,
    SYS_TRIGGERON,
    SYS_TRIGGERACT,
    SYS_TRIGRET,
    SYS_SLEEP,
    SYS_THREAD_CREATE,
    SYS_THREAD_EXIT,
    SYS_THREAD_JOIN,
    SYS_THREAD_CANCEL,
    SYS_THREAD_DETACH,
    SYS_GETTID,
    SYS_THREAD_TESTCANCEL,
    SYS_THREAD_CANCELSTATE,
    SYS_THREAD_CANCELTYPE,
    SYS_SCHED_YEILD,
    SYS_WAITQUE_CREATE,
    SYS_WAITQUE_DESTROY,
    SYS_WAITQUE_WAIT,
    SYS_WAITQUE_WAKE,
    SYS_PROC_RESERVED = 30,             /* 预留30个接口给进程管理 */
    SYS_HEAP,
    SYS_VMM_RESERVED = 40,              /* 预留10个接口给内存管理 */
    SYS_GETRES, 
    SYS_PUTRES,
    SYS_READRES, 
    SYS_WRITERES,
    SYS_CTLRES,
    SYS_DEVSCAN, 
    SYS_RES_RESERVED = 50,              /* 预留10个接口给资源管理 */
    SYS_ALARM,
    SYS_KTIME,
    SYS_GETTICKS,
    SYS_GETTIMEOFDAY,
    SYS_CLOCK_GETTIME,
    SYS_TIME_RESERVED = 60,             /* 预留10个接口给时间管理 */
    SYS_SRVCALL,
    SYS_SRVCALL_LISTEN,
    SYS_SRVCALL_ACK,
    SYS_SRVCALL_BIND,
    SYS_SRVCALL_UNBIND,
    SYSCALL_NR,
};
unsigned long __syscall0(unsigned long num);
unsigned long __syscall1(unsigned long num, unsigned long arg0);
unsigned long __syscall2(unsigned long num, unsigned long arg0,
        unsigned long arg1);
unsigned long __syscall3(unsigned long num, unsigned long arg0,
        unsigned long arg1, unsigned long arg2);
unsigned long __syscall4(unsigned long num, unsigned long arg0,
        unsigned long arg1, unsigned long arg2, unsigned long arg3);

/* 进行宏定义 */
#define syscall0(type, num) \
        (type) __syscall0((unsigned long ) num)

#define syscall1(type, num, arg0) \
        (type) __syscall1((unsigned long ) num, (unsigned long ) arg0)

#define syscall2(type, num, arg0, arg1) \
        (type) __syscall2((unsigned long ) num, (unsigned long ) arg0,\
        (unsigned long ) arg1)

#define syscall3(type, num, arg0, arg1, arg2) \
        (type) __syscall3((unsigned long ) num, (unsigned long ) arg0,\
        (unsigned long ) arg1, (unsigned long ) arg2)

#define syscall4(type, num, arg0, arg1, arg2, arg3) \
        (type) __syscall4((unsigned long ) num, (unsigned long ) arg0,\
        (unsigned long ) arg1, (unsigned long ) arg2, (unsigned long ) arg3)

#endif   /* _SYS_ */