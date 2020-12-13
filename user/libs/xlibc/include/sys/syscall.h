#ifndef _SYS_SYSCALL_H
#define _SYS_SYSCALL_H

#ifdef __cplusplus
extern "C" {
#endif
enum syscall_num {
    SYS_EXIT,
    SYS_FORK,
    SYS_WAITPID,
    SYS_GETPID,
    SYS_GETPPID,
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
    SYS_MUTEX_QUEUE_CREATE,
    SYS_MUTEX_QUEUE_DESTROY,
    SYS_MUTEX_QUEUE_WAIT,
    SYS_MUTEX_QUEUE_WAKE,
    SYS_PROC_RESERVED = 30,             /* 预留30个接口给进程管理 */
    SYS_HEAP,
    SYS_MUNMAP,
    SYS_VMM_RESERVED = 40,              /* 预留10个接口给内存管理 */
    SYS_DEVSCAN, 
    SYS_RES_RESERVED = 50,              /* 预留10个接口给资源管理 */
    SYS_ALARM,
    SYS_WALLTIME,
    SYS_GETTICKS,
    SYS_GETTIMEOFDAY,
    SYS_CLOCK_GETTIME,
    SYS_TIME_RESERVED = 60,             /* 预留10个接口给时间管理 */
    SYS_SRVCALL,
    SYS_SRVCALL_LISTEN,
    SYS_SRVCALL_ACK,
    SYS_SRVCALL_BIND,
    SYS_SRVCALL_UNBIND,
    SYS_SRVCALL_FETCH,
    SYS_UNID,
    SYS_TSTATE,
    SYS_GETVER,
    SYS_MSTATE,
    SYS_USLEEP,
    SYS_OPEN,
    SYS_CLOSE,
    SYS_READ,
    SYS_WRITE,
    SYS_LSEEK,
    SYS_ACCESS,
    SYS_UNLINK,
    SYS_FTRUNCATE,
    SYS_FSYNC,
    SYS_IOCTL,
    SYS_FCNTL,
    SYS_TELL,
    SYS_MKDIR,
    SYS_RMDIR,
    SYS_RENAME,
    SYS_GETCWD,
    SYS_CHDIR,
    SYS_EXECVE,
    SYS_STAT,
    SYS_FSTAT,
    SYS_CHMOD,
    SYS_FCHMOD,
    SYS_OPENDIR,
    SYS_CLOSEDIR,
    SYS_READDIR,
    SYS_REWINDDIR,
    SYS_MKFS,
    SYS_MOUNT,
    SYS_UNMOUNT,
    SYS_DUP,
    SYS_DUP2,
    SYS_PIPE,
    SYS_SHMGET,
    SYS_SHMPUT,
    SYS_SHMMAP,
    SYS_SHMUNMAP,
    SYS_SEMGET,
    SYS_SEMPUT,
    SYS_SEMDOWN,
    SYS_SEMUP,
    SYS_MSGGET,
    SYS_MSGPUT,
    SYS_MSGSEND,
    SYS_MSGRECV,
    SYS_PROBEDEV,
    SYS_EXPSEND,
    SYS_EXPCATCH,
    SYS_EXPBLOCK,
    SYS_EXPRET,
    SYS_OPENDEV,
    SYS_OPENFIFO,
    SYS_ACNTLOGIN,
    SYS_ACNTREGISTER,
    SYS_ACNTNAME,
    SYS_ACNTVERIFY,
    SYS_SOCKET,
    SYS_BIND,
    SYS_CONNECT,
    SYS_LISTEN,
    SYS_ACCEPT,
    SYS_SEND,
    SYS_RECV,
    SYS_SENDTO,
    SYS_RECVFROM,
    SYS_SHUTDOWN,
    SYS_GETPEERNAME,
    SYS_GETSOCKNAME,
    SYS_GETSOCKOPT,
    SYS_SETSOCKOPT,
    SYS_IOCTLSOCKET,
    SYS_SELECT,
    SYS_MMAP,
    SYS_CREATPROCESS,
    SYS_RESUMEPROCESS,
    SYS_CREATE_PORT,
    SYS_CLOSE_PORT,
    SYS_ACCEPT_PORT,
    SYS_CONNECT_PORT,
    SYS_RECEIVE_PORT,
    SYS_REPLY_PORT,
    SYS_REQUEST_PORT,
    SYSCALL_NR,
};

extern unsigned long __syscall0(unsigned long num);
extern unsigned long __syscall1(unsigned long num, unsigned long arg0);
extern unsigned long __syscall2(unsigned long num, unsigned long arg0,
        unsigned long arg1);
extern unsigned long __syscall3(unsigned long num, unsigned long arg0,
        unsigned long arg1, unsigned long arg2);
extern unsigned long __syscall4(unsigned long num, unsigned long arg0,
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

#ifdef __cplusplus
}
#endif

#endif   /* _SYS_ */