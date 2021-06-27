#ifndef _XBOOK_SYSCALL_H
#define _XBOOK_SYSCALL_H

#include <stdint.h>
#include <xbook/config.h>

typedef void * syscall_t;

#if defined(CONFIG_NEWSYSCALL)
#define SYS_exit 93
#define SYS_exit_group 94
#define SYS_getpid 172
#define SYS_kill 129
#define SYS_tkill 130
#define SYS_read 63
#define SYS_write 64
#define SYS_openat 56
#define SYS_close 57
#define SYS_lseek 62
#define SYS_brk 214
#define SYS_linkat 37
#define SYS_unlinkat 35
#define SYS_mkdirat 34
#define SYS_renameat 38
#define SYS_chdir 49
#define SYS_getcwd 17
#define SYS_fstat 80
#define SYS_fstatat 79
#define SYS_faccessat 48
#define SYS_pread 67
#define SYS_pwrite 68
#define SYS_uname 160
#define SYS_getuid 174
#define SYS_geteuid 175
#define SYS_getgid 176
#define SYS_getegid 177
#define SYS_mmap 222
#define SYS_munmap 215
#define SYS_mremap 216
#define SYS_mprotect 226
#define SYS_prlimit64 261
#define SYS_getmainvars 2011
#define SYS_rt_sigaction 134
#define SYS_writev 66
#define SYS_gettimeofday 169
#define SYS_times 153
#define SYS_fcntl 25
#define SYS_ftruncate 46
#define SYS_getdents 61
#define SYS_dup 23
#define SYS_readlinkat 78
#define SYS_rt_sigprocmask 135
#define SYS_ioctl 29
#define SYS_getrlimit 163
#define SYS_setrlimit 164
#define SYS_getrusage 165
#define SYS_clock_settime 112
#define SYS_clock_gettime 113
#define SYS_set_tid_address 96
#define SYS_set_robust_list 99

#define OLD_SYSCALL_THRESHOLD 1024
#define SYS_open 1024
#define SYS_link 1025
#define SYS_unlink 1026
#define SYS_mkdir 1030
#define SYS_access 1033
#define SYS_stat 1038
#define SYS_lstat 1039
#define SYS_time 1062

#define IS_ERR_VALUE(x) ((unsigned long)(x) >= (unsigned long)-4096)
#define ERR_PTR(x) ((void*)(long)(x))
#define PTR_ERR(x) ((long)(x))

/* add by my self */
#define SYS_getppid 173
#define SYS_sched_setaffinity 122
#define SYS_sched_getaffinity 123
#define SYS_sched_yield 124
#define SYS_sched_get_priority_max 125
#define SYS_sched_get_priority_min 126
#define SYS_umount2 39
#define SYS_mount 40
#define SYS_dup3 24
#define SYS_nanosleep 101
#define SYS_clone 220
#define SYS_pipe2 59
#define SYS_getdents64 61

#define SYS_mailread 0
#define SYS_mailwrite 0
#define SYS_spawn 0
#define SYS_execve 221
#define SYS_wait4 260

#define SYS_rt_sigreturn 139
#define SYS_alarm 27

#define SYS_getitimer 102
#define SYS_setitimer 103

#define SYS_setxattr 5
#define SYS_lsetxattr 6
#define SYS_fsetxattr 7
#define SYS_getxattr 8
#define SYS_lgetxattr 9
#define SYS_fgetxattr 10
#define SYS_listxattr 11
#define SYS_llistxattr 12
#define SYS_flistxattr 13
#define SYS_removexattr 14
#define SYS_lremovexattr 15
#define SYS_fremovexattr 16

#define SYS_setpriority 140
#define SYS_getpriority 141
#define SYS_reboot 142

#define SYS_setgid 144
#define SYS_setuid 146

#define SYS_getgroups 158
#define SYS_setgroups 159

#define SYS_setresuid 147
#define SYS_getresuid 148
#define SYS_setresgid 149
#define SYS_getresgid 150

#define SYS_setpgid 154
#define SYS_getpgid 155
#define SYS_gettid 178

#define SYS_set_tid_address 96

#define SYS_getsid 156
#define SYS_setsid 157
#define SYS_times 153

#define SYSCALL_NR 1100
#else
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
    SYS_SCHED_YIELD,
    SYS_MUTEX_QUEUE_CREATE,
    SYS_MUTEX_QUEUE_DESTROY,
    SYS_MUTEX_QUEUE_WAIT,
    SYS_MUTEX_QUEUE_WAKE,
    SYS_PROC_RESERVED = 30,             /* 预留30个接口给进程管理 */
    SYS_BRK,
    SYS_MUNMAP,
    SYS_VMM_RESERVED = 40,              /* 预留10个接口给内存管理 */
    SYS_SCANDEV, 
    SYS_RES_RESERVED = 50,              /* 预留10个接口给资源管理 */
    SYS_ALARM,
    SYS_WALLTIME,
    SYS_GETTICKS,
    SYS_GETTIMEOFDAY,
    SYS_CLOCK_GETTIME,
    SYS_TIME_RESERVED = 60,             /* 预留10个接口给时间管理 */
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
    SYS_ACNTLOGIN,
    SYS_ACNTREGISTER,
    SYS_ACNTNAME,
    SYS_ACNTVERIFY,
    SYS_MMAP,
    SYS_CREATPROCESS,
    SYS_RESUMEPROCESS,
    SYS_BIND_PORT,
    SYS_UNBIND_PORT,
    SYS_RECEIVE_PORT,
    SYS_REPLY_PORT,
    SYS_REQUEST_PORT,
    SYS_FASTIO,
    SYS_FASTREAD,
    SYS_FASTWRITE,
    SYS_EXPMASK,
    SYS_EXPHANDLER,
    SYS_SYSCONF,
    SYS_TIMES,
    SYS_GETHOSTNAME,
    SYS_GETPGID,
    SYS_SETPGID,
    SYS_MKFIFO,
    SYS_SOCKCALL,
    SYS_OPENAT,
    SYS_CLONE,    
    SYS_GETDENTS64,
    SYS_UNAME,
    SYSCALL_NR,
};
#endif

extern syscall_t syscalls[];

/* 属于检测点的系统调用有：
SYS_WAITPID，SYS_SLEEP，SYS_THREAD_JOIN，SYS_GETRES, SYS_PUTRES, SYS_READRES, 
SYS_WRITERES */

void syscall_init();
int syscall_error(uint32_t callno);

#endif   /*_XBOOK_SYSCALL_H*/
