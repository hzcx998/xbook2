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

...
```
