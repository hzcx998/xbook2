#include <xbook/syscall.h>
#include <xbook/process.h>
#include <xbook/memspace.h>
#include <xbook/alarm.h>
#include <xbook/clock.h>
#include <xbook/mutexqueue.h>
#include <xbook/fs.h>
#include <xbook/driver.h>
#include <xbook/sharemem.h>
#include <xbook/sem.h>
#include <xbook/msgqueue.h>
#include <xbook/walltime.h>
#include <xbook/account.h>
#include <xbook/portcomm.h>
#include <xbook/kernel.h>
#include <xbook/schedule.h>
#include <xbook/fifo.h>
#include <xbook/sockcall.h>
#include <xbook/signal.h>
#include <sys/uname.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <dirent.h>

syscall_t syscalls[SYSCALL_NR];

#if defined(CONFIG_NEWSYSCALL)
typedef unsigned long (*syscall_func_t)(
    unsigned long,
    unsigned long,
    unsigned long,
    unsigned long,
    unsigned long,
    unsigned long,
    void *); 
#else
typedef unsigned long (*syscall_func_t)(
    unsigned long,
    unsigned long,
    unsigned long,
    unsigned long,
    void *); 
#endif

void syscall_default()
{
    errprint("user call a not supported syscall!\n");
    exception_raise(EXP_CODE_SYS);
}

void syscall_init()
{
    int i; for (i = 0; i < SYSCALL_NR; i++) {
        syscalls[i] = syscall_default;
    }

    #if defined(CONFIG_NEWSYSCALL)
    syscalls[SYS_openat] = sys_openat;
    syscalls[SYS_open] = sys_open;
    syscalls[SYS_close] = sys_close;
    syscalls[SYS_read] = sys_read;
    syscalls[SYS_write] = sys_write;
    syscalls[SYS_ioctl] = sys_ioctl;
    syscalls[SYS_getpid] = sys_get_pid;
    syscalls[SYS_getppid] = sys_get_ppid;
    syscalls[SYS_sched_yield] = sys_sched_yield;
    syscalls[SYS_clone] = sys_clone;
    syscalls[SYS_exit] = sys_exit;
    syscalls[SYS_wait4] = sys_waitpid;
    syscalls[SYS_execve] = sys_execve;
    syscalls[SYS_times] = sys_times;
    syscalls[SYS_gettimeofday] = sys_gettimeofday;
    syscalls[SYS_nanosleep] = sys_nanosleep;
    syscalls[SYS_mmap] = sys_mmap;
    syscalls[SYS_munmap] = sys_munmap;
    syscalls[SYS_fstat] = sys_fstat;
    // syscalls[SYS_linkat] = ;
    syscalls[SYS_unlinkat] = sys_unlinkat;
    syscalls[SYS_uname] = sys_uname;
    syscalls[SYS_brk] = sys_brk;
    syscalls[SYS_getcwd] = sys_getcwd;
    syscalls[SYS_chdir] = sys_chdir;
    syscalls[SYS_mkdirat] = sys_mkdirat;
    syscalls[SYS_getdents64] = sys_getdents;
    syscalls[SYS_pipe2] = sys_pipe;
    syscalls[SYS_dup] = sys_dup;
    syscalls[SYS_dup3] = sys_dup2;
    syscalls[SYS_mount] = sys_mount;
    syscalls[SYS_umount2] = sys_unmount;
    syscalls[SYS_rt_sigaction] = sys_rt_sigaction;
    syscalls[SYS_rt_sigprocmask] = sys_rt_sigprocmask;
    syscalls[SYS_rt_sigreturn] = sys_rt_sigreturn;
    syscalls[SYS_alarm] = sys_alarm;
    syscalls[SYS_setitimer] = sys_setitimer;
    syscalls[SYS_getitimer] = sys_getitimer;
    #else
    syscalls[SYS_EXIT] = sys_exit;
    syscalls[SYS_FORK] = sys_fork;
    syscalls[SYS_WAITPID] = sys_waitpid;
    syscalls[SYS_GETPID] = sys_get_pid;
    syscalls[SYS_GETPPID] = sys_get_ppid;
    syscalls[SYS_SLEEP] = sys_sleep;
    #ifdef CONFIG_PTHREAD
    syscalls[SYS_THREAD_CREATE] = sys_thread_create;
    syscalls[SYS_THREAD_EXIT] = sys_thread_exit;
    syscalls[SYS_THREAD_JOIN] = sys_thread_join;
    syscalls[SYS_THREAD_DETACH] = sys_thread_detach;
    syscalls[SYS_GETTID] = sys_get_tid;
    syscalls[SYS_THREAD_CANCEL] = sys_thread_cancel;
    syscalls[SYS_THREAD_TESTCANCEL] = sys_thread_testcancel;
    syscalls[SYS_THREAD_CANCELSTATE] = sys_thread_setcancelstate;
    syscalls[SYS_THREAD_CANCELTYPE] = sys_thread_setcanceltype;
    #endif
    syscalls[SYS_SCHED_YIELD] = sys_sched_yield;
    #ifdef CONFIG_PTHREAD
    syscalls[SYS_MUTEX_QUEUE_CREATE] = sys_mutex_queue_alloc;
    syscalls[SYS_MUTEX_QUEUE_DESTROY] = sys_mutex_queue_free;
    syscalls[SYS_MUTEX_QUEUE_WAIT] = sys_mutex_queue_wait;
    syscalls[SYS_MUTEX_QUEUE_WAKE] = sys_mutex_queue_wake;
    #endif
    syscalls[SYS_BRK] = sys_brk;
    syscalls[SYS_MUNMAP] = sys_munmap;
    syscalls[SYS_ALARM] = sys_alarm;
    syscalls[SYS_WALLTIME] = sys_get_walltime;
    syscalls[SYS_GETTICKS] = sys_get_ticks;
    syscalls[SYS_GETTIMEOFDAY] = sys_gettimeofday;
    syscalls[SYS_CLOCK_GETTIME] = sys_clock_gettime;
    syscalls[SYS_UNID] = sys_unid;
    syscalls[SYS_TSTATE] = sys_tstate;
    syscalls[SYS_GETVER] = sys_getver;
    syscalls[SYS_MSTATE] = sys_mstate;    
    syscalls[SYS_USLEEP] = sys_usleep;    
    syscalls[SYS_OPEN] = sys_open;
    syscalls[SYS_CLOSE] = sys_close;
    syscalls[SYS_READ] = sys_read;
    syscalls[SYS_WRITE] = sys_write;
    syscalls[SYS_LSEEK] = sys_lseek;
    syscalls[SYS_ACCESS] = sys_access;
    syscalls[SYS_UNLINK] = sys_unlink;
    syscalls[SYS_FTRUNCATE] = sys_ftruncate;
    syscalls[SYS_FSYNC] = sys_fsync;
    syscalls[SYS_IOCTL] = sys_ioctl;
    syscalls[SYS_FCNTL] = sys_fcntl;
    syscalls[SYS_TELL] = sys_tell;
    syscalls[SYS_MKDIR] = sys_mkdir;
    syscalls[SYS_RMDIR] = sys_rmdir;
    syscalls[SYS_RENAME] = sys_rename;
    syscalls[SYS_CHDIR] = sys_chdir;
    syscalls[SYS_GETCWD] = sys_getcwd;
    syscalls[SYS_EXECVE] = sys_execve;
    syscalls[SYS_STAT] = sys_stat;
    syscalls[SYS_FSTAT] = sys_fstat;
    syscalls[SYS_CHMOD] = sys_chmod;
    syscalls[SYS_FCHMOD] = sys_fchmod;
    syscalls[SYS_OPENDIR] = sys_opendir;
    syscalls[SYS_CLOSEDIR] = sys_closedir;
    syscalls[SYS_READDIR] = sys_readdir;
    syscalls[SYS_REWINDDIR] = sys_rewinddir;
    syscalls[SYS_MKFS] = sys_mkfs;
    syscalls[SYS_MOUNT] = sys_mount;
    syscalls[SYS_UNMOUNT] = sys_unmount;
    syscalls[SYS_DUP] = sys_dup;
    syscalls[SYS_DUP2] = sys_dup2;
    syscalls[SYS_PIPE] = sys_pipe;
    syscalls[SYS_SHMGET] = sys_shmem_get;
    syscalls[SYS_SHMPUT] = sys_shmem_put;
    syscalls[SYS_SHMMAP] = sys_shmem_map;
    syscalls[SYS_SHMUNMAP] = sys_shmem_unmap;
    syscalls[SYS_SEMGET] = sys_sem_get;
    syscalls[SYS_SEMPUT] = sys_sem_put;
    syscalls[SYS_SEMDOWN] = sys_sem_down;
    syscalls[SYS_SEMUP] = sys_sem_up;
    syscalls[SYS_MSGGET] = sys_msgque_get;
    syscalls[SYS_MSGPUT] = sys_msgque_put;
    syscalls[SYS_MSGSEND] = sys_msgque_send;
    syscalls[SYS_MSGRECV] = sys_msgque_recv;
    syscalls[SYS_PROBEDEV] = sys_probedev;
    syscalls[SYS_EXPSEND] = sys_expsend;
    syscalls[SYS_EXPCATCH] = sys_expcatch;
    syscalls[SYS_EXPBLOCK] = sys_expblock;
    syscalls[SYS_EXPRET] = sys_excetion_return;
    #ifdef CONFIG_ACCOUNT
    syscalls[SYS_ACNTLOGIN] = sys_account_login;
    syscalls[SYS_ACNTREGISTER] = sys_account_register;
    syscalls[SYS_ACNTNAME] = sys_account_name;
    syscalls[SYS_ACNTVERIFY] = sys_account_verify;
    #endif
    syscalls[SYS_MMAP] = sys_mmap;
    syscalls[SYS_CREATPROCESS] = sys_create_process;
    syscalls[SYS_RESUMEPROCESS] = sys_resume_process;
    syscalls[SYS_BIND_PORT] = sys_port_comm_bind;
    syscalls[SYS_UNBIND_PORT] = sys_port_comm_unbind;
    syscalls[SYS_RECEIVE_PORT] = sys_port_comm_receive;
    syscalls[SYS_REPLY_PORT] = sys_port_comm_reply;
    syscalls[SYS_REQUEST_PORT] = sys_port_comm_request;
    syscalls[SYS_SCANDEV] = sys_scandev;
    syscalls[SYS_FASTIO] = sys_fastio;
    syscalls[SYS_FASTREAD] = sys_fastread;
    syscalls[SYS_FASTWRITE] = sys_fastwrite;
    syscalls[SYS_EXPMASK] = sys_expmask;
    syscalls[SYS_EXPHANDLER] = sys_exphandler;
    syscalls[SYS_SYSCONF] = sys_sysconf;
    syscalls[SYS_TIMES] = sys_times;
    syscalls[SYS_GETHOSTNAME] = sys_gethostname;
    syscalls[SYS_GETPGID] = sys_get_pgid;
    syscalls[SYS_SETPGID] = sys_set_pgid;
    syscalls[SYS_MKFIFO] = sys_mkfifo;
    #if 0
    syscalls[SYS_SOCKCALL] = sys_sockcall;
    #endif
    syscalls[SYS_OPENAT] = sys_openat;
    syscalls[SYS_CLONE] = sys_clone;
    syscalls[SYS_GETDENTS64] = sys_getdents;
    syscalls[SYS_UNAME] = sys_uname;
    #endif 
}

int syscall_check(uint32_t callno)
{
    if (callno >= SYSCALL_NR) {
        keprint(PRINT_ERR "syscall: bad number %d, raise exception!\n", callno);
        exception_raise(EXP_CODE_SYS);
        return 1;
    }
    return 0;
}

unsigned long syscall_dispatch(trap_frame_t *frame)
{
    task_t *cur = task_current;
    /* 开始统计时间 */
    cur->syscall_ticks_delta = sys_get_ticks();
    // TODO: call different func in different arch
    unsigned long retval;
    #if defined(__X86__)
    syscall_func_t func = (syscall_func_t)syscalls[frame->eax];
    retval = func(frame->ebx, frame->ecx, frame->esi,
                            frame->edi, frame);
    #endif
    #if defined(__RISCV64__)
    /* 
    a7: syscall number
    a0: arg0
    a1: arg1
    a2: arg1
    a3: arg3
    a4: arg4
    a5: arg5
    a0-a1: retval
    */
    syscall_func_t func = (syscall_func_t)syscalls[frame->a7];
    //dbgprintln("[syscall] syscall number %d", frame->a7);

    #if defined(CONFIG_NEWSYSCALL)
    retval = func(frame->a0, frame->a1, frame->a2,
                            frame->a3, frame->a4, frame->a5, frame);
    #else
    retval = func(frame->a0, frame->a1, frame->a2,
                            frame->a3, frame);
    #endif
    frame->a0 = retval;
    #endif
    /* 结束统计时间 */
    cur->syscall_ticks_delta = sys_get_ticks() - cur->syscall_ticks_delta;
    cur->syscall_ticks += cur->syscall_ticks_delta;
    return retval;
}