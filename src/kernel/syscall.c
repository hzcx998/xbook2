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
#include <xbook/config.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <dirent.h>

syscall_t syscalls[SYSCALL_NR];

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
    syscalls[SYS_EXIT] = sys_exit;
    syscalls[SYS_FORK] = sys_fork;
    syscalls[SYS_WAITPID] = sys_waitpid;
    syscalls[SYS_GETPID] = sys_get_pid;
    syscalls[SYS_GETPPID] = sys_get_ppid;
    syscalls[SYS_SLEEP] = sys_sleep;
    syscalls[SYS_THREAD_CREATE] = sys_thread_create;
    syscalls[SYS_THREAD_EXIT] = sys_thread_exit;
    syscalls[SYS_THREAD_JOIN] = sys_thread_join;
    syscalls[SYS_THREAD_DETACH] = sys_thread_detach;
    syscalls[SYS_GETTID] = sys_get_tid;
    syscalls[SYS_THREAD_CANCEL] = sys_thread_cancel;
    syscalls[SYS_THREAD_TESTCANCEL] = sys_thread_testcancel;
    syscalls[SYS_THREAD_CANCELSTATE] = sys_thread_setcancelstate;
    syscalls[SYS_THREAD_CANCELTYPE] = sys_thread_setcanceltype;
    syscalls[SYS_SCHED_YEILD] = sys_sched_yeild;
    syscalls[SYS_MUTEX_QUEUE_CREATE] = sys_mutex_queue_alloc;
    syscalls[SYS_MUTEX_QUEUE_DESTROY] = sys_mutex_queue_free;
    syscalls[SYS_MUTEX_QUEUE_WAIT] = sys_mutex_queue_wait;
    syscalls[SYS_MUTEX_QUEUE_WAKE] = sys_mutex_queue_wake;
    syscalls[SYS_HEAP] = sys_mem_space_expend_heap;
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
    syscalls[SYS_OPENDEV] = sys_opendev;
    syscalls[SYS_OPENFIFO] = sys_openfifo;
    syscalls[SYS_ACNTLOGIN] = sys_account_login;
    syscalls[SYS_ACNTREGISTER] = sys_account_register;
    syscalls[SYS_ACNTNAME] = sys_account_name;
    syscalls[SYS_ACNTVERIFY] = sys_account_verify;
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
}

int syscall_error(uint32_t callno)
{
    if (callno >= SYSCALL_NR) {
        keprint(PRINT_ERR "syscall: bad number %d, raise exception!\n", callno);
        exception_raise(EXP_CODE_SYS);
        return 1;
    }
    return 0;
}
