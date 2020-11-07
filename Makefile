# MIT License
# Copyright (c) 2020 Jason Hu, Zhu Yu
all:

# tools
MAKE		= make
FATFS_DIR	= tools/fatfs

# System environment variable.
ifeq ($(OS),Windows_NT)
	FATFS_BIN		:= fatfs
else
	FATFS_BIN		:= $(FATFS_DIR)/fatfs
endif

TRUNC		= truncate
RM			= rm
DD			= dd
MKDIR		= mkdir

# virtual machine
QEMU 		= qemu-system-i386

# images and rom
IMAGE_DIR	= develop/image
FLOPPYA_IMG	= $(IMAGE_DIR)/a.img
HDA_IMG		= $(IMAGE_DIR)/c.img
HDB_IMG		= $(IMAGE_DIR)/d.img
ROM_DIR		= develop/rom

BOOT_DISK	= $(FLOPPYA_IMG)
FS_DISK		= $(HDB_IMG)

# image size
FLOPPYA_SZ	= 1474560  # 1.44 MB
HDA_SZ		= 33554432 # 32 MB
HDB_SZ		= 33554432

# environment dir
LIBRARY_DIR	= ./library
SYSTEM_DIR	= ./system
USER_DIR	= ./user

#kernel disk
LOADER_OFF 	= 2
LOADER_CNTS = 8

SETUP_OFF 	= 10
SETUP_CNTS 	= 90

KERNEL_OFF 	= 100
KERNEL_CNTS	= 1024		# assume 512kb 

# arch dir

KERNSRC		= src
ARCH	= $(KERNSRC)/arch/x86

# kernel boot binary
BOOT_BIN 	= $(ARCH)/boot/boot.bin
LOADER_BIN 	= $(ARCH)/boot/loader.bin
SETUP_BIN 	= $(ARCH)/boot/setup.bin

# kernel file
KERNEL_ELF 	= $(KERNSRC)/kernel.elf

# 参数
.PHONY: all kernel build debuild qemu qemudbg lib sys usr

# 默认所有动作，编译内核后，把引导、内核、init服务、文件服务和rom文件写入磁盘
all : kernel 
	$(DD) if=$(BOOT_BIN) of=$(BOOT_DISK) bs=512 count=1 conv=notrunc
	$(DD) if=$(LOADER_BIN) of=$(BOOT_DISK) bs=512 seek=$(LOADER_OFF) count=$(LOADER_CNTS) conv=notrunc
	$(DD) if=$(SETUP_BIN) of=$(BOOT_DISK) bs=512 seek=$(SETUP_OFF) count=$(SETUP_CNTS) conv=notrunc
	$(DD) if=$(KERNEL_ELF) of=$(BOOT_DISK) bs=512 seek=$(KERNEL_OFF) count=$(KERNEL_CNTS) conv=notrunc
	$(FATFS_BIN) $(FS_DISK) $(ROM_DIR) 0

# run启动虚拟机
run: qemu

# 先写rom，在编译内核
kernel:
	@$(MAKE) -s -C  ./src

clean:
	@$(MAKE) -s -C ./src clean

# 构建环境。镜像>工具>环境>rom
build:
	-$(MKDIR) $(IMAGE_DIR)
	-$(MKDIR) $(ROM_DIR)/bin
	-$(MKDIR) $(ROM_DIR)/sbin
	-$(MKDIR) $(ROM_DIR)/usr
	$(TRUNC) -s $(FLOPPYA_SZ) $(FLOPPYA_IMG)
	$(TRUNC) -s $(HDA_SZ) $(HDA_IMG)
	$(TRUNC) -s $(HDB_SZ) $(HDB_IMG) 
ifeq ($(OS),Windows_NT)
else
	$(MAKE) -s -C  $(FATFS_DIR)
endif
	$(MAKE) -s -C  $(LIBRARY_DIR)
	$(MAKE) -s -C  $(SYSTEM_DIR)
	$(MAKE) -s -C  $(USER_DIR)
	$(FATFS_BIN) $(FS_DISK) $(ROM_DIR) 0

# 清理环境。
debuild: 
	$(MAKE) -s -C  $(KERNSRC) clean
ifeq ($(OS),Windows_NT)
else
	$(MAKE) -s -C  $(FATFS_DIR) clean
endif
	$(MAKE) -s -C  $(LIBRARY_DIR) clean
	$(MAKE) -s -C  $(SYSTEM_DIR) clean
	$(MAKE) -s -C  $(USER_DIR) clean
	-$(RM) -r $(ROM_DIR)/bin
	-$(RM) -r $(ROM_DIR)/sbin
	-$(RM) -r $(ROM_DIR)/usr
	-$(RM) -r $(IMAGE_DIR)
	
# 重新编译所有库
lib: 
	$(MAKE) -s -C  $(LIBRARY_DIR)

lib_c: 
	$(MAKE) -s -C  $(LIBRARY_DIR) clean
	
# 重新编译所有系统程序
sys: 
	$(MAKE) -s -C  $(SYSTEM_DIR)

sys_c: 
	$(MAKE) -s -C  $(SYSTEM_DIR) clean

# 不清理编译
usr:
	$(MAKE) -s -C  $(USER_DIR)
	
usr_c:
	$(MAKE) -s -C  $(USER_DIR) clean
	
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

# 磁盘配置：
#	1. IDE DISK：-hda $(HDA_IMG) -hdb $(HDB_IMG) \
# 	2. AHCI DISK: -drive id=disk0,file=$(HDA_IMG),if=none \
		-drive id=disk1,file=$(HDB_IMG),if=none \
		-device ahci,id=ahci \
		-device ide-drive,drive=disk0,bus=ahci.0 \
		-device ide-drive,drive=disk1,bus=ahci.1 \


QEMU_ARGUMENT = -m 512M \
		-name "XBOOK Development Platform for x86" \
		-fda $(FLOPPYA_IMG) \
		-hda $(HDA_IMG) -hdb $(HDB_IMG) \
		-boot a \
		-serial stdio \
		-soundhw sb16 \
		-soundhw pcspk

#		-fda $(FLOPPYA_IMG) -hda $(HDA_IMG) -hdb $(HDB_IMG) -boot a \
#		-net nic,model=rtl8139 -net tap,ifname=tap0,script=no,downscript=no 

# qemu启动
qemu: all
	$(QEMU) $(QEMU_ARGUMENT)

# 调试配置：-S -gdb tcp::10001,ipv4
qemudbg: all
	$(QEMU) -S -gdb tcp::10001,ipv4 $(QEMU_ARGUMENT)
