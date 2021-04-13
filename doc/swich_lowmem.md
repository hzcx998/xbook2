# 内核起始虚拟地址切换
内核支持在高端地址和低端地址。  
高端地址在0x80000000的地方，低端地址在0的地方。  
低端地址支持是为了支持grub而采取的措施。  
切换需要进行配置，步骤如下：  
1. 修改src/arch/x86/kernel.ld中的虚拟地址为0x00100000
2. 修改src/include/xbook/config.h中CONFIG_KERN_LOWMEM为1
3. 修改src/arch/x86/include/arch/const.inc的KERNEL_LOWMEM为1
4. 修改src/arch/x86/boot/page.h中KERN_LOWMEN为1
5. 修改libs/xlibc/arch/x86/user.ld中的虚拟地址为0x80000000