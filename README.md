# xbook2
xbook2是xbook内核的第二个版本，这个版本，尝试把原来的宏内核修改成混内核。添加用户态服务，并把内核部分功能转移到用户态，
以减小内核大小。

# 核心结构
KERNEL: vmm, task, ipc, drivers  
USER: service, libOS, user program  

# 来源
vmm, task, ipc , service 来自微内核 Microkernel.   
drivers 来自宏内核 Monolithic kernel.  
libOS 来自外内核 exokernel.  

因此，这是一个基于传统内核的新混内核。目前只是尝试一些新的内容，因此可能会比较鸡肋。

正在开发中，敬请期待吧！ :)  
-2020.3.16 胡自成（一个不知名的喜欢搞系统开发的男同学）