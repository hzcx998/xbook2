# xbook2操作系统内核
xbook2操作系统内核是一个基于intel x86平台的32位处理器的系统内核，可运行在qemu，bochs，virtual box，vmware等虚拟机中。也可以在物理机上面运行（需要有系统支持的驱动才行）

xbook2被设计成一个跨处理器平台的架构，有ARCH目录，可以在里面添加一个新的处理器平台。不过目前也是尽量将平台相关的分离到arch里面，并为完全分离出来，待后面多实现几个平台后，才能更好的让处理器平台和内核部分进行更优化的分离。  

内核结构示意图：
```
USER MODE:
+---------------------------+
|  libs | apps          |   
+---------------------------+
KERNEL MODE: 
+---------------------------+  
| FSAL                      |
+---------------------------+
+---------------------------+  
|task | ipc | vmm | drivers |
+---------------------------+
|            arch           | 
+---------------------------+
\                           /
+---------------------------+
|          hardware         |
+---------------------------+
```

| 目录            | 描述                                      |
| ------------- | --------------------------------------- |
| develop       | 开发时用到的磁盘镜像，ROM文件系统内容等   |
| doc           | 操作系统相关的文档                               |
| scripts       | 用到的xbuild脚本和其它配置文件 |
| src           | xbook2内核的源码                  |
| tools         | 内核开发需要用到的工具                         |
| user          | 和用户相关的内容，库，应用程序等  |

系统功能列表：
```
多进程，内核多线程，用户多线程
虚拟内存管理，分页内存管理，物理内存管理
管道通信，共享内存，消息队列，信号量

IDE硬盘驱动，PS/2鼠标，键盘驱动，VBE视频驱动
UART串口驱动

FATFS文件系统
PTHREAD线程库
```

开发环境准备（Windows/Linux）：  
```
整体思路：
1. 用git从仓库克隆源码或者直接下载源码。
2. 配置最基础的工具集：gcc, nasm, ld, dd, rm, objdump, objcopy。
3. 配置虚拟机：qemu（默认），bochs，virtual box， vmware任选其一。
5. 进入xbook2的根目录目录，打开终端或者命令行，输入命令make build先构建环境，然后make run编译运行。
```

## Windows环境搭建

1.下载我提取的工具包：[BuildTools](https://gitee.com/hzc1998/bookos-web-db/blob/master/tools/BuildTools.zip), 下载后配置解压目录环境变量到系统环境变量Path里面。（注意，如果你的电脑上已经有mingw或者cygwin环境，请把这个工具包的环境变量放到靠前的位置，不然不会执行工具包里面的程序）

2.下载qemu最新版：[Qemu](https://www.qemu.org/) 下载后安装，配置安装目录环境变量到系统环境变量Path里面，或者下载我提取的版本：[Qemu-i386](https://gitee.com/hzc1998/bookos-web-db/blob/master/tools/Qemu-i386.rar)，下载后配置解压目录环境变量到系统环境变量Path里面。

3.如果想要用虚拟机加速，下载windows下面的qemu加速扩展程序[HAXM](https://github.com/intel/haxm/releases) v7.6.5：，下载后安装即可.


## Linux环境搭建
```
1.安装gcc, nasm: 
    Ubuntu/Kali Linux: sudo apt-get install gcc nasm
    Red hat/Fedora/Centos: sudo yum install gcc nasm
    
2.安装qemu虚拟机：
    Ubuntu/Kali Linux: sudo apt-get install qemu-system-x86
    Red hat/Fedora/Centos: sudo yum install qemu-system-x86
    
```

# Mac环境搭建

```bash
# 1.安装 i386-gcc
> brew tap nativeos/i386-elf-toolchain
> brew install i386-elf-binutils i386-elf-gcc
# 2.安装 truncate nasm qemu 
> brew install truncate nasm qemu
> make build
> make qemu
```

## 编译时build后直接run即可（可加-jn参数多线程编译，n是线程数。）：

```bash
> make build    # 构建环境
> make run      # 编译并运行，默认使用qemu虚拟机运行
```

## 编译时可用的命令：
```bash
> make          # 只编译源码
> make build    # 构建环境
> make debuild  # 清理环境
> make run      # 编译并运行，默认使用qemu虚拟机运行
> make qemu     # 使用qemu虚拟机运行
> make clean    # 清除编译产生的对象文件以及可执行文件
> make user     # 只编译用户程序（在开发应用时常用）
> make user_clean     # 只清除用户态生成的内容
```

联系方式：
开源官网：www.book-os.org  
E-mail: book_os@163.com  
个人邮箱：2323168280@qq.com  
个人QQ: 2323168280  
开发交流QQ群：913813452  