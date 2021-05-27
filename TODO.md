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
* 2021/5/11
* riscv64开发流程：物理内存管理[ok]->分页机制[ok]->中断管理[ok]
* 2021/5/18
* riscv64使用标准task结构来管理多任务 [ok]
* 实现fork[ok], exec, exit[ok], wait[ok], sleep[ok]等进程相关的功能
* 2021/5/25
* 进程管理相关测试 [ok]
* 内存管理相关测试 [ok]
* 文件系统相关测试 [ok]
* 时间相关测试 [ok]
* 其它相关测试
* 添加控制台输入控制，这样就可以有按键输入了。[ok]
* exec对参数的支持。libc库对参数的支持。[ok]
* riscv64中没有实现对envp的传入。
* 添加cpu驱动 [ok]
* 优化内存分配与释放，查找内存漏洞。
* 尝试移植到k210板子上面。
* 添加sdcard驱动[ok]
* 修复不能启动用户程序的bug。(库程序导致)
* 使用比赛标准库，编译init以及sh。
* 需要实现的系统调用如下：  
当在主机实现所有系统调用后，需要进行适配，**参数数值**以及**系统调用**号的修改
```
    [ok] int open(const char *path, int flags)
    [ok] int openat(int dirfd,const char *path, int flags)
    
    [ok] ssize_t read(int fd, void *buf, size_t len)
    [ok] ssize_t write(int fd, const void *buf, size_t len)
    [ok] int brk(void *addr)
    [ok] int chdir(const char *path)
    [no] pid_t clone(int (*fn)(void *arg), void *arg, void *stack,
                    size_t stack_size, unsigned long flags)
    [ok] int close(int fd)
    [ok] int dup(int fd)
    [ok] int dup2(int old, int new)
    [no] int execve(const char *pathname, char *const argv[], char *const envp[])
    [bug] 执行->execve->printf->__stdio_write: page fauilt: user pid=7 name=test_echo access unmaped vir_mem area.
    [ok] void exit(int code)
    [ok] pid_t fork(void)
    [ok] int fstat(int fd, struct kstat *st)
```