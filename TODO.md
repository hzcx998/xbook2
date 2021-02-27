# 期望列表
* 添加USB总线协议
* 添加显卡驱动
* 添加USB鼠标，键盘
* 添加USB磁盘
* 任务切换时，对浮点寄存器也要切换
* 添加SMP多处理器
* 添加ARM平台
* 添加RSCV平台
* 添加amd64平台

# 小目标
* 2020/10/23
* 添加硬盘引导，支持FAT32文件系统引导
* 用linux制作一个安装包，把镜像文件刻录到某个磁盘，如果磁盘不存在数据，则进行格式化文件系统。

* 2020/10/26
* 驱动管理：添加驱动程序高级部分处理，添加磁盘缓存功能。优化并完善框架之上的内容。尝试添加热插播功能。

* 2020/11/3
* 需添加：磁盘换页机制
* 2020/12/31
* 编写文档，描述内核结构
* 2021/2/22
* 移植bash，把缺少的函数补上，不做具体实现。
    {
    接口：sys/times, unistd/id.c,tcgetattr, tcsetattr,pwd.c
        gethostname, umask, termios.c,mkfifo
    修改:signames.h改成bookos的，
    处理：SIGTSTP
    bug on bash: locale.c->loca_shiftstates
    修复：bash管道不能使用。不能执行shell脚本。
    计划：补充空函数，完善后在尝试进行修复。
    }
* 2021/2/23
* 移植coreutils
* 2021/2/24
* 添加ac97声卡驱动，添加声音框架，通用声卡处理接口。处理原始文件。
* grub引导启动
