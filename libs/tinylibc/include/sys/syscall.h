#ifndef __SYSCALL_H__
#define __SYSCALL_H__

#include "../../arch/riscv/syscall_arch.h"
#include "../../arch/riscv/syscall_ids.h"

#ifndef __scc
#define __scc(X) ((long)(X))
typedef long syscall_arg_t;
#endif

#define __syscall1(n, a) __syscall1(n, __scc(a))
#define __syscall2(n, a, b) __syscall2(n, __scc(a), __scc(b))
#define __syscall3(n, a, b, c) __syscall3(n, __scc(a), __scc(b), __scc(c))
#define __syscall4(n, a, b, c, d) __syscall4(n, __scc(a), __scc(b), __scc(c), __scc(d))
#define __syscall5(n, a, b, c, d, e) __syscall5(n, __scc(a), __scc(b), __scc(c), __scc(d), __scc(e))
#define __syscall6(n, a, b, c, d, e, f) __syscall6(n, __scc(a), __scc(b), __scc(c), __scc(d), __scc(e), __scc(f))

#define __SYSCALL_NARGS_X(a, b, c, d, e, f, g, h, n, ...) n
#define __SYSCALL_NARGS(...) __SYSCALL_NARGS_X(__VA_ARGS__, 7, 6, 5, 4, 3, 2, 1, 0, )
#define __SYSCALL_CONCAT_X(a, b) a##b
#define __SYSCALL_CONCAT(a, b) __SYSCALL_CONCAT_X(a, b)
#define __SYSCALL_DISP(b, ...)                        \
    __SYSCALL_CONCAT(b, __SYSCALL_NARGS(__VA_ARGS__)) \
    (__VA_ARGS__)

#define __syscall(...) __SYSCALL_DISP(__syscall, __VA_ARGS__)
#define syscall(...) __syscall(__VA_ARGS__)

#endif // __SYSCALL_H

// See LICENSE for license details.

#ifndef _PK_SYSCALL_H
#define _PK_SYSCALL_H

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

#define AT_FDCWD -100

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

#define SYS_sethostname 161

#define SYS_getrlimit 163
#define SYS_setrlimit 164
#define SYS_prlimit64 261

#define SYS_umask 166
#define SYS_prctl 167
#define SYS_adjtimex 171
#define SYS_sysinfo 179

#endif
