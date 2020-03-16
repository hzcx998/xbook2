# xbook2
xbook2 is a new kernel for book os, it is based on meta-kernel. It's the second version of xbook kernel.  

# meta-kernel ?
After studying the existing operating system kernels, I myself came up with a new kernel structure, the meta-kernel.  

At present, it only deals with the theoretical state, which has not been proved by practice.  

So, I wrote down this project to prove it, xbook is my previous operating system project, this time I implemented xbook2 meta-kernel based on xbook!

# Special!
KERNEL: vmm, task, ipc, drivers(module)  
USER: libOS, libc, user program and other libs.  

vmm, task, ipc came from Microkernel.   
drivers(module) came from Monolithic kernel.  
libOS came from exokernel.  

So it is a new kernel based on the traditional kernel, the meta-kernel.  

Stay tuned! :)  
-2020.3.16 Jason Hu