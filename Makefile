# MIT License
# Copyright (c) 2020 Jason Hu, Zhu Yu
all:

# tools
MAKE		= make
FATFS_DIR	= tools/fatfs
FATFS_BIN	= $(FATFS_DIR)/fatfs
TRUNC		= truncate
RM			= rm
DD			= dd

# virtual machine
QEMU 		= qemu-system-i386

# images and rom
FLOPPYA_IMG	= develop/image/a.img
HDA_IMG		= develop/image/c.img
HDB_IMG		= develop/image/d.img
ROM_DIR		= develop/rom

# image size
FLOPPYA_SZ	= 1474560
HDA_SZ		= 10321920
HDB_SZ		= 10321920

# environment dir
LIBARY_DIR	= libary
SERVICE_DIR	= service
USER_DIR	= user

#kernel disk
LOADER_OFF 	= 2		
LOADER_CNTS = 8

SETUP_OFF 	= 10		
SETUP_CNTS 	= 90

KERNEL_OFF 	= 100
KERNEL_CNTS	= 512		# assume 512 kb, now just 256kb 

# arch dir
KERSRC		= src
ARCH 		= $(KERSRC)/arch/x86
# kernel boot binary
BOOT_BIN 	= $(ARCH)/boot/boot.bin
LOADER_BIN 	= $(ARCH)/boot/loader.bin
SETUP_BIN 	= $(ARCH)/boot/setup.bin

# kernel file
KERNEL_ELF 	= $(KERSRC)/kernel.elf

# 参数
.PHONY: all kernel build debuild rom qemu qemudbg lib srv usr

# 默认所有动作
all : kernel 
	$(DD) if=$(BOOT_BIN) of=$(FLOPPYA_IMG) bs=512 count=1 conv=notrunc
	$(DD) if=$(LOADER_BIN) of=$(FLOPPYA_IMG) bs=512 seek=$(LOADER_OFF) count=$(LOADER_CNTS) conv=notrunc
	$(DD) if=$(SETUP_BIN) of=$(FLOPPYA_IMG) bs=512 seek=$(SETUP_OFF) count=$(SETUP_CNTS) conv=notrunc
	$(DD) if=$(KERNEL_ELF) of=$(FLOPPYA_IMG) bs=512 seek=$(KERNEL_OFF) count=$(KERNEL_CNTS) conv=notrunc
	$(FATFS_BIN) $(HDB_IMG) $(ROM_DIR) 10

# run启动虚拟机
run: qemu

# 先写rom，在编译内核
kernel:
	@$(MAKE) -s -C ./src

# 构建环境。镜像>工具>环境>rom
build:
	$(TRUNC) -s $(FLOPPYA_SZ) $(FLOPPYA_IMG)
	$(TRUNC) -s $(HDA_SZ) $(HDA_IMG)
	$(TRUNC) -s $(HDA_SZ) $(HDB_IMG) 
	cd $(FATFS_DIR) && $(MAKE)
	cd $(LIBARY_DIR) && $(MAKE)
	cd $(SERVICE_DIR) && $(MAKE)
	cd $(USER_DIR) && $(MAKE)
	$(FATFS_BIN) $(HDB_IMG) $(ROM_DIR) 10

# 清理环境。
debuild: 
	cd $(KERSRC) && make clean
	cd $(FATFS_DIR) && $(MAKE) clean
	cd $(LIBARY_DIR) && $(MAKE) clean
	cd $(SERVICE_DIR) && $(MAKE) clean
	cd $(USER_DIR) && $(MAKE) clean
	-$(RM) $(FLOPPYA_IMG)
	-$(RM) $(HDA_IMG)
	-$(RM) $(HDB_IMG)

# 写入rom
rom: 
	$(FATFS_BIN) $(HDB_IMG) $(ROM_DIR) 10

# 重新编译所有库
lib: 
	cd $(LIBARY_DIR) && $(MAKE) clean
	cd $(LIBARY_DIR) && $(MAKE)

# 重新编译所有服务
srv: 
	cd $(SERVICE_DIR) && $(MAKE) clean
	cd $(SERVICE_DIR) && $(MAKE)

# 重新编译所有用户
usr: 
	cd $(SERVICE_DIR) && $(MAKE) clean
	cd $(SERVICE_DIR) && $(MAKE)


#-hda $(HDA_IMG) -hdb $(HDB_IMG)
# 网卡配置: 
#	-net nic,vlan=0,model=rtl8139,macaddr=12:34:56:78:9a:be
# 网络模式：
#	1.User mode network(Slirp) :User网络
#		-net user
#	2.Tap/tun network : Tap网络
#		-net tap
# 		-net tap,vlan=0,ifname=tap0
#	example: -net nic,model=rtl8139 -net tap,ifname=tap0,script=no,downscript=no 
		
# 音频配置：
# 	a.使用蜂鸣器：-soundhw pcspk
#	b.使用声霸卡：-soundhw sb16
# 控制台串口调试： -serial stdio 
QEMU_ARGUMENT = -m 256M \
		-name "Xbook Development Platform for x86" \
		-fda $(FLOPPYA_IMG) -hda $(HDA_IMG) -hdb $(HDB_IMG) -boot a \
		-serial stdio

#		-net nic,model=rtl8139 -net tap,ifname=tap0,script=no,downscript=no 

# qemu启动
qemu: all
	$(QEMU) $(QEMU_ARGUMENT)

# 调试配置：-S -gdb tcp::10001,ipv4
qemudbg: all
	$(QEMU) -S -gdb tcp::10001,ipv4 $(QEMU_ARGUMENT)
