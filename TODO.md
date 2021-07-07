# 期望列表
* 添加USB总线协议
* 添加显卡驱动
* 添加USB鼠标，键盘
* 添加USB磁盘
* 添加SMP多处理器
* 添加ARM平台
* 添加RSCV平台
* 添加amd64平台
* 2020/10/23
* 添加硬盘引导，支持FAT32文件系统引导
* 用linux制作一个安装包，把镜像文件刻录到某个磁盘，如果磁盘不存在数据，则进行格式化文件系统。
* 2020/11/3
* 需添加：磁盘换页机制
* 2020/12/31
* 编写文档，描述内核结构
* 2021/2/22
* 移植bash，把缺少的函数补上，不做具体实现。
```
    接口：unistd/id.c,tcgetattr, tcsetattr
        umask, termios.c
    处理：SIGTSTP
    计划：补充空函数，完善后 在尝试进行修复。
```
* 2021/2/23
* 移植coreutils
* 2021/2/24
* 添加ac97声卡驱动，添加声音框架，通用声卡处理接口。处理原始文件。
* 完善sb16声卡驱动。
* grub引导启动
* 2021/4/24
* 完善网络机制，添加local socket套接字支持，添加网络支持相关的库
```
1. 完善协议栈
2. 实现各种网络操作库
3. 实现简单的网络应用程序
4. 实现连接外网
5. 实现local socket
6. 完善网络相关的其余模块
```
* 2021/4/26
* 完善LPC，支持共享内存映射，为大区域数据传输打好基础
* 2021/4/29
* 把内核文件进行压缩，然后再引导中解压，最后跳转进去执行。
* 优化内核代码，并编写开发文档。

# 2021/6/23
## busybox系统调用完成情况
```
[ok] __NR_setitimer 103
[ok] __NR_clock_settime 112
[ok] __NR_clock_gettime 113
[no] __NR_syslog 116                # syslog依赖于socket套接字，UNIX本地套接字
[ok] __NR_sched_setaffinity 122
[ok] __NR_sched_getaffinity 123
[ok] __NR_sched_yield 124
[ok] __NR_sched_get_priority_max 125
[ok] __NR_sched_get_priority_min 126
[ok] __NR_kill 129
[ok] __NR_tkill 130
[ok] __NR_rt_sigaction 134
[ok] __NR_rt_sigprocmask 135
[ok] __NR_rt_sigreturn 139
[ok] SYS_setxattr 5
[ok] SYS_lsetxattr 6
[ok] SYS_fsetxattr 7
[ok] SYS_getxattr 8
[ok] SYS_lgetxattr 9
[ok] SYS_fgetxattr 10
[ok] SYS_listxattr 11
[ok] SYS_llistxattr 12
[ok] SYS_flistxattr 13
[ok] SYS_removexattr 14
[ok] SYS_lremovexattr 15
[ok] SYS_fremovexattr 16
[ok] __NR_setpriority 140
[ok] __NR_getpriority 141
[ok] __NR_reboot 142
# 完成所有和id相关的系统调用
[ok] __NR_setgid 144
[ok] __NR_setuid 146
[ok] __NR_getgroups 158
[ok] __NR_setgroups 159
[ok] __NR_getresuid 148
[ok] __NR_getresgid 150
[ok] __NR_getuid 174
[ok] __NR_geteuid 175
[ok] __NR_getgid 176
[ok] __NR_getegid 177
[ok] __NR_setpgid 154
[ok] __NR_getpgid 155
[ok] __NR_getsid 156
[ok] __NR_setsid 157
[ok] __NR_getpid 172
[ok] __NR_getppid 173
[ok] __NR_gettid 178
[ok] __NR_set_tid_address 96
[ok] __NR_times 153
[ok] __NR_uname 160
[ok] __NR_sethostname 161
[ok] __NR_getrlimit 163
[ok] __NR_setrlimit 164
[ok] __NR_prlimit64 261
[ok] __NR_umask 166
[ok] __NR_prctl 167             : 使用到的参数：PR_SET_NO_NEW_PRIVS，PR_SET_PDEATHSIG
[ok] __NR_gettimeofday 169
[ok] __NR_getcwd 17
[ok] __NR_adjtimex 171
[ok] __NR_sysinfo 179

# 可以只实现接口，不是先具体内容
[no] li a7,186 	 __NR_msgget 186
[no] li a7,187 	 __NR_msgctl 187
[no] li a7,190 	 __NR_semget 190
[no] li a7,191 	 __NR_semctl 191
[no] li a7,193 	 __NR_semop 193
[no] li a7,194 	 __NR_shmget 194
[no] li a7,195 	 __NR_shmctl 195
[no] li a7,196 	 __NR_shmat 196
[no] li a7,197 	 __NR_shmdt 197
[no] li a7,198 	 __NR_socket 198
[no] li a7,199 	 __NR_socketpair 199
[no] li a7,200 	 __NR_bind 200
[no] li a7,201 	 __NR_listen 201
[no] li a7,204 	 __NR_getsockname 204
[no] li a7,205 	 __NR_getpeername 205
[no] li a7,208 	 __NR_setsockopt 208
[no] li a7,209 	 __NR_getsockopt 209
[no] li a7,210 	 __NR_shutdown 210
[no] li a7,90 	 __NR_capget 90
[no] li a7,91 	 __NR_capset 91

## 需要实现接口以及内容
[ok] li a7,213 	 __NR_readahead 213
[ok] li a7,214 	 __NR_brk 214
[ok] li a7,215 	 __NR_munmap 215
[no] li a7,216 	 __NR_mremap 216, # 测试案例中并没有使用
[ok] li a7,220 	 __NR_clone 220
[ok] li a7,221 	 __NR_execve 221
[ok] li a7,222 	 __NR3264_mmap 222
[ok] li a7,224 	 __NR_swapon 224
[ok] li a7,225 	 __NR_swapoff 225
[ok] li a7,226 	 __NR_mprotect 226
[ok] li a7,228 	 __NR_mlock 228
[ok] li a7,229 	 __NR_munlock 229
[ok] li a7,233 	 __NR_madvise 233
[ok] li a7,23 	 __NR_dup 23
[ok] li a7,24 	 __NR_dup3 24
[ok] li a7,25 	 __NR3264_fcntl 25
[ok] li a7,260 	 __NR_wait4 260
[ok] li a7,266 	 __NR_clock_adjtime 266
[ok] li a7,267 	 __NR_syncfs 267
[ok] li a7,268 	 __NR_setns 268
[ok] li a7,276 	 __NR_renameat2 276
[ok] li a7,29 	 __NR_ioctl 29
[ok] li a7,32 	 __NR_flock 32

[no] li a7,33 	 __NR_mknodat 33
[no] li a7,34 	 __NR_mkdirat 34
[no] li a7,35 	 __NR_unlinkat 35
[no] li a7,36 	 __NR_symlinkat 36
[no] li a7,37 	 __NR_linkat 37
[no] li a7,39 	 __NR_umount2 39
[no] li a7,40 	 __NR_mount 40
[no] li a7,41 	 __NR_pivot_root 41
[no] li a7,43 	 __NR3264_statfs 43
[no] li a7,46 	 __NR3264_ftruncate 46
[no] li a7,47 	 __NR_fallocate 47
[no] li a7,48 	 __NR_faccessat 48
[no] li a7,49 	 __NR_chdir 49
[no] li a7,5 	 __NR_setxattr 5
[no] li a7,50 	 __NR_fchdir 50
[no] li a7,51 	 __NR_chroot 51
[no] li a7,52 	 __NR_fchmod 52
[no] li a7,53 	 __NR_fchmodat 53
[no] li a7,54 	 __NR_fchownat 54
[no] li a7,55 	 __NR_fchown 55
[no] li a7,56 	 __NR_openat 56
[no] li a7,57 	 __NR_close 57
[no] li a7,59 	 __NR_pipe2 59
[no] li a7,6 	 __NR_lsetxattr 6
[no] li a7,61 	 __NR_getdents64 61
[no] li a7,62 	 __NR3264_lseek 62
[no] li a7,63 	 __NR_read 63
[no] li a7,64 	 __NR_write 64
[no] li a7,65 	 __NR_readv 65
[no] li a7,66 	 __NR_writev 66
[no] li a7,71 	 __NR3264_sendfile 71
[no] li a7,73 	 __NR_ppoll 73
[no] li a7,78 	 __NR_readlinkat 78
[no] li a7,79 	 __NR3264_fstatat 79
[no] li a7,80 	 __NR3264_fstat 80
[no] li a7,81 	 __NR_sync 81
[no] li a7,88 	 __NR_utimensat 88
[no] li a7,89 	 __NR_acct 89
[no] li a7,92 	 __NR_personality 92
[no] li a7,93 	 __NR_exit 93
[no] li a7,94 	 __NR_exit_group 94
[no] li a7,97 	 __NR_unshare 97
[no] li a7,98 	 __NR_futex 98
...
```
