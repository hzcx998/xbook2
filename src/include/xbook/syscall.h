#ifndef _XBOOK_SYSCALL_H
#define _XBOOK_SYSCALL_H

/*
系统调用的作用是，用户可以通过内核暴露的系统调用
来执行内核提供的部分操作。
对于元内核，至少需要为用户提供进程，内存管理，进程间通信，时间管理，设备管理
相应的系统调用接口。

当同一类型的事物多样复杂时，使用资源管理接口。
由于是以一种资源的形式提供给用户，所以，命名就很重要了。
类型+名字的形式是比较不错的。

资源管理：可以被用户直接使用的资源。
控制管理：用与对进程的执行的管理和控制。

进程管理：
fork
execr
execf
exit
wait
getpid
getppid
trigger
triggeron
triggeract
...

内存管理：
heap

时间管理：
标准c时间管理方式。

IPC和设备：
getres(type, name, flags)
puttres(res)
ctlres(res, cmd, arg)
readres(res, arg, buffer, size)
writeres(res, arg, buffer, size)

内核其它资源：

*/

typedef void * syscall_t;


enum {
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
    SYS_PROC_RESERVED = 20,             /* 预留20个接口给进程管理 */
    SYS_HEAP,
    SYS_VMM_RESERVED = 30,              /* 预留10个接口给内存管理 */
    SYS_GETRES, 
    SYS_PUTRES,
    SYS_READRES, 
    SYS_WRITERES,
    SYS_CTLRES,
    SYS_RES_RESERVED = 40,              /* 预留10个接口给资源管理 */
    SYS_ALARM,
    SYS_KTIME,
    SYS_TIME_RESERVED = 60,             /* 预留20个接口给时间管理 */
    SYSCALL_NR,
};
void init_syscall();

#endif   /*_XBOOK_SYSCALL_H*/
