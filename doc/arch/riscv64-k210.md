# riscv64 for k210 in xbook2 kernel
xbook2内核的riscv64-k210是基于[xv6-riscv](https://github.com/mit-pdos/xv6-riscv)和[xv6-k210](https://github.com/HUST-OS/xv6-k210)修改的，最开始找资料时，发现其代码简洁，简单易懂，并且有效，于是就选择这两个开源项目作为主要参考资料，在此，感谢这两个项目的作者的开源。

xbook2是一个宏内核，支持内核多线程，多进程的系统内核。内存管理采取了类似于buddy物理内存的算法，memcache来管理内核虚拟内存。驱动io框架是参考windowsNT内核驱动框架开发。文件系统采用的类似于VFS的FSAL（文件系统抽象层）管理机制，目前支持FAT32文件系统，/dev设备文件系统。

## ARCH（src/arch）
arch相关的都在src/arch目录下面，这里面存放着不同平台不同的内容，引导，中断，物理内存管理，分页机制都在arch里面。

mach-qemu是虚拟机调试时的代码，mach-k210是k210板子的一些代码，主要代码都放在mach-qemu中，由于涉及到的区别不是很多，所以，参考xv6-k210的实现方式，用QEMU宏来区别qemu环境和k210环境中的差异。

### 1. boot（src/arch/mach-qemu/boot）
引导程序采用的是[RUSTSBI](https://github.com/luojia65/rustsbi)，可以通过它来初始化一些执行环境，然后为我们内核提供一些基础的接口，便于内核的开发。目前使用的是xv6-k210编译的RUSTSBI-QEMU和RUSTSBI-K210

在RISCV中，有3种执行模式，M(Machine),S(Supervisor),U(User)模式，RUSTSBI位于M模式，拥有最高的优先级权限，xbook2内核位于S模式，应用程序位于U模式。
RUSTSBI启动后会跳转到我们自己的kernel中执行，然后我们可以通过RUSTSBI提供的接口（通过寄存器传参+ecall的方式）来调用，我们可以利用这些接口来简化内核开发。

### 2. entry.S（src/arch/mach-qemu/entry.S）
内核的入口在_start这个标签（mach-qemu/entry.S）,在里面初始化栈之后，就进入c语言程序arch_init中。

### 3. arch.c（src/arch/mach-qemu/arch.c）
这里面执行一些和arch相关的初始化，最开始初始化调试，然后初始化物理内存管理，分页机制，中断管理，然后跳转到更高层次抽象的内核中。

### 4. 调试输出（src/arch/mach-qemu/debug.c）
借助于SBI接口sbi_console_putchar来进行串口输出。

### 5. 物理内存管理（src/arch/mach-qemu/mm/mempool.c）
一个简单的类似于buddy的算法，使用数组+链表实现。最终提供API给其他模块使用。
```
page_alloc_normal(count)： 一般的页分配,count为页数量
page_alloc_user(count)：   用户程序使用的页
page_alloc_dma(count)：    dma使用的页
page_free(addr)：         释放已分配的页，addr为页地址
```
### 6. 分页管理（src/arch/mach-qemu/mm/page&page2.c）
分页机制中，有一个内核页目录表kernel_pgdir，存放了内核的虚拟地址映射表项，内核线程共享这个页表。初始化时，需要把内核程序本身以及一些内存映射，比如CLINT和PLIC的物理地址也要映射到虚拟地址中，不然开启分页后，访问物理地址会出现页故障。还有一个特殊的页，TRAMPOLINE，这个是用户态和内核态切换的切换代码，这个虚拟地址的代码是从用户态陷入内核态时就会跳转过去执行，从内核态返回时，也会从这个地方返回到用户态。

除此之外，还提供了页映射do_map_pages以及取消映射do_unmap_pages的机制，来使得每个进程都运行在自己的虚拟地址空间中。

还有页故障处理机制，当进程访问了不存在的虚拟地址，或者非法地址，就会在异常产生后跳转到页故障处理中，如果是可以修复的，比如说是正常的缺页，就重新映射并返回，如果是不可处理的缺页或者是没有对该页的访问权限，则会产生段故障，导致进程终止。

### 7. 中断管理（src/arch/mach-qemu/interrpt/）
中断管理主要分为外部中断管理和内部中断管理，外部中断管理主要是像串口，磁盘，SD卡中断的捕捉处理。内部中断主要是时钟中断，异常，环境调用中断的处理。内部中断的异常管理主要是当执行了错误的程序时产生，一般都是直接panic，页故障的话还可以进行适当处理后恢复正常。环境调用是拿来做系统调用的，就是当用户态执行call时，进行捕捉，然后根据寄存器传参，去调用对应的系统调用程序。
外部中断管理，采取了hardirq和softirq相结合的机制。类似于linux里面的中断上半部分和下半部分。

### 8. 驱动程序（src/arch/mach-qemu/drivers/）
和平台相关的驱动程序将会放到各个平台的子目录下面。而通用的驱动程序，比如ramdisk，null,zero等程序则是放到了src/drivers目录，而不是mach-xxx/srivers目录下面。
riscv平台目前有console驱动，cpu驱动，sdcard驱动，virtio_disk驱动。都是一些基础的驱动，能够满足最基础的内核环境。

## INIT（src/init/main.c）
当arch执行完成后，就会跳转到init中执行，这里面对内核的其他内容进行初始化，虚拟内存管理，多任务，文件系统，驱动框架等。
## VMM（src/vmm/）
虚拟内存管理中，主要分为内核虚拟地址管理和用户进程虚拟地址管理。内核在执行过程中也需要大量的内存才能运行，于是采用了比较高效的memcache来进行管理。提供了内存分配与释放接口。
```
mem_alloc(size):  分配内存
mem_free(addr):   释放内存
```
对进程地址空间的管理，采用vmm结构来进行管理，对每个进程的代码段，数据段，堆栈，都用不同的memspace内存空间来进行管理。

每个memspace都描述了一个内存空间的开始和结束，以及属性。比如栈空间从0x7fffe000开始到0x7ffff000结束，具有可读可写的用户属性。如果进行栈操作时，越界了，则会产生页故障异常。比如典型的fib函数执行时，就会消耗大量的栈。

## KERNEL（src/kernel/）
这里面存放的是内核的其他内容，比如时间管理，定时器，调试，异常，驱动框架，高级中断管理等内容。
驱动框架是根据WINDOWNT内核的框架来写的，是一个面向对象化的设计，有点复杂，但是用起来也不错。

### 系统调用（src/kernel/syscall.c）
xbook2内核中用一个数组来存放系统调用函数入口，每个入口存放一个内核功能函数，比如sys_open,sys_close,sys_brk等。用户通过ecall环境调用，将会产生一个内核异常，进入异常后，会根据a7寄存器的值来判断是哪个系统调用，然后再跳转过去执行，a0-a6存放了寄存器的参数。通过系统调用，用户态程序可以执行内核的一些功能，内核通过这个机制来保证自己的安全，同时也为用户提供了功能接口，就相当于RUSTSBI凌驾于KERNEL之上，KERNEL凌驾于USER之上一样。当然，如果USER要捣乱也是可以的，就想KERNEL也可以捣乱一样。

## TASK（src/task/）
内核支持多线程，以及多进程，同时也对用户态多线程给予了一定的支持，内核直接支持pthread部分函数，可以让用户态实现线程更加方便。
内核实现了一个基于时间片+优先级的调度算法，比较简单实用。内核也实现了任务的等待，退出，阻塞，唤醒等基本的功能。使用spinlock,mutexlock,semaphore来对线程的互斥访问进行保障。不仅支持fork+execve这套创建任务的模式，而且还支持create_process这种直接创建一个进程的模式，可以更高效得创建进程。
## IPC（src/ipc/）
进程间通信支持了fifo命名管道，pipe无名管道，msgqueue消息队列，sharemem共享内存，sem用户信号量，以及portcomm端口通信。这些都是传统的进程间通信机制。portcomm机制可以实现端到端的通信，可以拿来实现用户态的服务程序，更高效得执行服务。
## DRIVERS（src/drivers/）
通用驱动/抽象层驱动存放的路径。多是基于内存的驱动，比如ramdisk，tty,zero,null驱动。可以在不同的平台都可以使用，不过会根据内存的情况进行调整，是否使用，如果内存太小，就没有使用ramdisk的必要了。抽象层驱动是需要基于物理驱动，比如tty驱动需要基于控制台，串口，之类的驱动二次构建。

xbook2的驱动驱动框架参考winNT内核构建，它也具有open/close,read/write,ioctl这套框架，他是基于driver object和device object来进行管理的。一个driver对象可以对应多个device对象。比如一个disk驱动可以对应多个磁盘设备。也就是disk_driver->disk device0，disk device1，..disk deviceN这样。winNT驱动是支持热插拨的，而且看起来也很不错，但是其内部原理没有搞清楚，所以就没有实现，以后如果有需要，可以实现热插拔，这样使用起来也更方便和有效。

驱动框架还有一点就是对中断的管理和捕捉处理，采用了linux的软中断softirq机制，来快速的响应中断，以及有效地提高实时性。

## FS（src/fs/）
文件系统采取了类似于VFS虚拟文件系统的FSAL文件系统抽象层机制。在这个机制下面，也可以适配不同的文件系统，而且提供统一的接口。一个优势就是，每个文件系统可以独立实现，然后嵌入进来，可以完全不用以来文件系统提供的接口来独立实现。每个文件系统只需要写一个嵌入的接口就能够在FSAL下面跑了。
目前支持[FATFS](http://elm-chan.org/fsw/ff/00index_e.html)文件系统，这是一个开源项目，支持FAT12,FAT16,FAT32,EXFAT文件系统格式。
除此之外，还支持了/dev设备文件系统，可以把设备文件纳入统一的管理路径中来。

## LIB（src/lib/）
实现了常用的内存操作memset等，以及字符串strlen等函数，辅助内核实现。

## 总结
xbook2内核在riscv64平台上的移植，很大部分归功于xv6-riscv个xv6-k210这两个开源项目，在此再次感谢开源！xbook2的内核实现，有些没有走linux那套，比如对vfs的支持，这导致驱动框架也会不一样，以及对内存和磁盘交换的实现都会变得复杂。看起来是个不太明智的选择，但是它很酷！

在做这个riscv64-k210项目的这个2个月，感觉过得很充实，每天都像打鸡血一样投入开发，也很累，但是最后的结果还算让人满意，希望这个开源项目也对其他人有所帮助吧，就想别人的项目对我有帮助一样！

-2021/5/31

[回到首页](../../README.md)