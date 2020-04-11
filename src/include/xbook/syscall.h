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
getres(type, name, flags)
puttres(res)
ctlres(res, cmd, arg)
readres(res, arg, size)
writeres(res, arg, size)

资源管理：可以被用户直接使用的资源。
控制管理：用与对进程的执行的管理和控制。

进程管理：
proc_fork
proc_exec
proc_exit
proc_wait
proc_pid
...

内存管理：
vmm_heap

时间管理：
标准c时间管理方式。

进程间通信：
res = getres(IPC_SEM, "sem", IPC_CREAT)
ctlres(res, SEM_DOWN, 0)
ctlres(res, SEM_TRYDOWN, 0)
ctlres(res, SEM_UP, 0)
putres(res)

res = getres(IPC_SHM, "shm", IPC_CREAT)
ctlres(res, SHM_MAP, 0)
ctlres(res, SEM_DEL, 0)
putres(res)

res = getres(IPC_TRIG, "lsoft", 0)
ctlres(res, SHM_ACTIVE, 0)
putres(res)

ipcid = getipc(name, flags, arg)
putipc(ipcid)
ctlipc(ipcid, cmd, arg)

ctlipc(semid, SEM_DOWN, IPC_NOWAIT or 0);
ctlipc(semid, SEM_UP, 0);

ctlipc(semid, SHM_MAP, shmaddr);
ctlipc(semid, SHM_UNMAP, shmaddr);

writeres(res, 0, shmaddr, 0);
writeres(res, 0, 0, shmaddr);

readres(res, 0, shmaddr, 0);

writeres(res, IPC_NOWAIT or 0, 0, 0);
readres(res, 0, 0, 0);

readres(res, shmaddr, 0, 0);

writeres(res, msgflg, buf, size_t msgsz);
readres(res, msgflg, msgtype + msgbuf , size_t msgsz);

设备：
res = getres(DEV_CHR, "con0", DEV_RD | DEV_WR)
resbuf.off = 0;
resbuf.buf = 0;
writeres(res, &resbuf, 32);
putres(res);




内核其它资源：

*/

typedef void * syscall_t;

enum {
    SYS_EXIT,
    SYS_FORK,
    SYS_EXECR,
    SYS_EXECF,
    SYS_WAIT,
    SYS_COEXIST,
    SYS_GETPID,
    SYS_GETPPID,
    SYS_PROC_RESERVED = 20,             /* 预留20个接口给进程管理 */
    SYS_HEAP,
    SYS_VMM_RESERVED = 30,              /* 预留10个接口给内存管理 */
    SYS_GETRES, 
    SYS_PUTRES,
    SYS_READRES, 
    SYS_WRITERES,
    SYS_CTLRES,
    SYS_RES_RESERVED = 40,              /* 预留10个接口给资源管理 */
    SYS_TIME_RESERVED = 60,             /* 预留20个接口给时间管理 */
    SYSCALL_NR,
};
void init_syscall();

#endif   /*_XBOOK_SYSCALL_H*/
