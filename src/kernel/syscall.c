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
#include <xbook/power.h>
#include <xbook/uid.h>
#include <xbook/net.h>
#include <sys/uname.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/timex.h>
#include <sys/sysinfo.h>
#include <sys/select.h>
#include <dirent.h>
#include <errno.h>

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
    syscalls[SYS_exit_group] = sys_exit_group;
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
    syscalls[SYS_sched_setaffinity] = sys_sched_setaffinity;
    syscalls[SYS_sched_getaffinity] = sys_sched_getaffinity;
    syscalls[SYS_clock_gettime] = sys_clock_gettime;
    syscalls[SYS_clock_settime] = sys_clock_settime;
    syscalls[SYS_sched_get_priority_max] = sys_sched_get_priority_max;
    syscalls[SYS_sched_get_priority_min] = sys_sched_get_priority_min;
    syscalls[SYS_kill] = sys_kill;
    syscalls[SYS_tkill] = sys_tkill;
    syscalls[SYS_removexattr] = sys_removexattr;
    syscalls[SYS_lremovexattr] = sys_lremovexattr;
    syscalls[SYS_fremovexattr] = sys_fremovexattr;
    syscalls[SYS_setxattr] = sys_setxattr;
    syscalls[SYS_lsetxattr] = sys_lsetxattr;
    syscalls[SYS_fsetxattr] = sys_fsetxattr;
    syscalls[SYS_getxattr] = sys_getxattr;
    syscalls[SYS_lgetxattr] = sys_lgetxattr;
    syscalls[SYS_fgetxattr] = sys_fgetxattr;
    syscalls[SYS_listxattr] = sys_listxattr;
    syscalls[SYS_llistxattr] = sys_llistxattr;
    syscalls[SYS_flistxattr] = sys_flistxattr;
    syscalls[SYS_setpriority] = sys_setpriority;
    syscalls[SYS_getpriority] = sys_getpriority;
    syscalls[SYS_reboot] = sys_reboot;
    syscalls[SYS_getuid] = sys_getuid;
    syscalls[SYS_geteuid] = sys_geteuid;
    syscalls[SYS_getgid] = sys_getgid;
    syscalls[SYS_getegid] = sys_getegid;
    syscalls[SYS_setuid] = sys_setuid;
    syscalls[SYS_setgid] = sys_setgid;
    syscalls[SYS_setgroups] = sys_setgroups;
    syscalls[SYS_getgroups] = sys_getgroups;

    syscalls[SYS_setresuid] = sys_setresuid;
    syscalls[SYS_getresuid] = sys_getresuid;
    syscalls[SYS_setresgid] = sys_setresgid;
    syscalls[SYS_getresgid] = sys_getresgid;
    
    syscalls[SYS_setpgid] = sys_set_pgid;
    syscalls[SYS_getpgid] = sys_get_pgid;
    syscalls[SYS_gettid] = sys_get_tid;
    syscalls[SYS_set_tid_address] = sys_set_tid_address;

    syscalls[SYS_setsid] = sys_setsid;
    syscalls[SYS_getsid] = sys_getsid;
    
    syscalls[SYS_times] = sys_times;
    syscalls[SYS_sethostname] = sys_sethostname;

    syscalls[SYS_getrlimit] = sys_getrlimit;
    syscalls[SYS_setrlimit] = sys_setrlimit;
    syscalls[SYS_prlimit64] = sys_prlimit;

    syscalls[SYS_umask] = sys_umask;
    syscalls[SYS_prctl] = sys_prctl;
    syscalls[SYS_adjtimex] = sys_adjtimex;
    syscalls[SYS_sysinfo] = sys_sysinfo;
    syscalls[SYS_readahead] = sys_readahead;
    syscalls[SYS_swapon] = sys_swapon;
    syscalls[SYS_swapoff] = sys_swapoff;

    syscalls[SYS_mprotect] = sys_mprotect;
    syscalls[SYS_mlock] = sys_mlock;
    syscalls[SYS_munlock] = sys_munlock;
    syscalls[SYS_madvise] = sys_madvise;
    
    syscalls[SYS_clock_adjtime] = sys_clock_adjtime;
    syscalls[SYS_syncfs] = sys_syncfs;
    syscalls[SYS_setns] = sys_setns;
    syscalls[SYS_renameat2] = sys_renameat;
    syscalls[SYS_flock] = sys_flock;
    syscalls[SYS_mknodat] = sys_mknodat;
    syscalls[SYS_symlinkat] = sys_symlinkat;
    syscalls[SYS_linkat] = sys_linkat;
    syscalls[SYS_fchmod] = sys_fchmod;
    
    syscalls[SYS_readv] = sys_readv;
    syscalls[SYS_writev] = sys_writev;

    syscalls[SYS_tstate] = sys_tstate;


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
    syscalls[SYS_GETPRIORITY] = sys_getpriority;
    syscalls[SYS_SETPRIORITY] = sys_setpriority;
    
    syscalls[SYS_GETRLIMIT] = sys_getrlimit;
    syscalls[SYS_SETRLIMIT] = sys_setrlimit;
    syscalls[SYS_PRLIMIT64] = sys_prlimit;

    syscalls[SYS_SYMLINKAT] = sys_symlinkat;
    syscalls[SYS_LINKAT] = sys_linkat;
    syscalls[SYS_GETITIMER] = sys_getitimer;
    syscalls[SYS_SETITIMER] = sys_setitimer;
    syscalls[SYS_NANOSLEEP] = sys_nanosleep;
    syscalls[SYS_SELECT] = sys_select;
    syscalls[SYS_SCHED_GET_PRIORITY_MAX] = sys_sched_get_priority_max;
    syscalls[SYS_SCHED_GET_PRIORITY_MIN] = sys_sched_get_priority_min;
    syscalls[SYS_SYNC] = sys_sync;

    syscalls[SYS_MPROTECT] = sys_mprotect;
    syscalls[SYS_MSYNC] = sys_msync;
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

void debug_syscall(trap_frame_t *frame)
{
    dbgprintln("[syscall] syscall number %d", frame->a7);
    dbgprintln("arg0 %lx, %d", frame->a0, frame->a0);
    dbgprintln("arg1 %lx, %d", frame->a1, frame->a1);
    dbgprintln("arg2 %lx, %d", frame->a2, frame->a2);
    dbgprintln("arg3 %lx, %d", frame->a3, frame->a3);
    dbgprintln("arg4 %lx, %d", frame->a4, frame->a4);
    dbgprintln("arg5 %lx, %d", frame->a5, frame->a5);
    dbgprintln("arg6 %lx, %d", frame->a6, frame->a6);    
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
    // debug_syscall(frame);
    #if defined(CONFIG_NEWSYSCALL)
    retval = func(frame->a0, frame->a1, frame->a2,
                            frame->a3, frame->a4, frame->a5, frame);
    #else
    retval = func(frame->a0, frame->a1, frame->a2,
                            frame->a3, frame);
    #endif
    frame->a0 = retval;
    #endif
    if (retval == (int) -ENOSYS) {
        errprint("[syscall] no:%d not supported now!\n", retval);
    }
    /* 结束统计时间 */
    cur->syscall_ticks_delta = sys_get_ticks() - cur->syscall_ticks_delta;
    cur->syscall_ticks += cur->syscall_ticks_delta;
    return retval;
}