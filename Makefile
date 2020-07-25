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
MKDIR		= mkdir

# virtual machine
QEMU 		= qemu-system-i386

# images and rom
IMAGE_DIR	= develop/image
FLOPPYA_IMG	= $(IMAGE_DIR)/a.img
HDA_IMG		= $(IMAGE_DIR)/c.img
HDB_IMG		= $(IMAGE_DIR)/d.img
ROM_DIR		= develop/rom

# image size
FLOPPYA_SZ	= 1474560
HDA_SZ		= 40321920
HDB_SZ		= 10321920

# 默认大小为10M
ROM_DISK_SZ	= 10

# environment dir
LIBRARY_DIR	= ./library
SERVICE_DIR	= ./service
USER_DIR	= ./user

#kernel disk
LOADER_OFF 	= 2		
LOADER_CNTS = 8

SETUP_OFF 	= 10		
SETUP_CNTS 	= 90

KERNEL_OFF 	= 100
KERNEL_CNTS	= 512		# assume 256kb 

FILESRV_OFF 	= 700
FILESRV_CNTS	= 512		# assume 256kb 

# arch dir

KERNSRC		= src
ARCH	= $(KERNSRC)/arch/x86

# kernel boot binary
BOOT_BIN 	= $(ARCH)/boot/boot.bin
LOADER_BIN 	= $(ARCH)/boot/loader.bin
SETUP_BIN 	= $(ARCH)/boot/setup.bin

# kernel file
KERNEL_ELF 	= $(KERNSRC)/kernel.elf

# service file
FILESRV_BIN	= $(ROM_DIR)/sbin/filesrv

# 参数
.PHONY: all kernel build debuild rom qemu qemudbg lib srv usr

# 默认所有动作，编译内核后，把引导、内核、init服务、文件服务和rom文件写入磁盘
all : kernel 
	$(DD) if=$(BOOT_BIN) of=$(FLOPPYA_IMG) bs=512 count=1 conv=notrunc
	$(DD) if=$(LOADER_BIN) of=$(FLOPPYA_IMG) bs=512 seek=$(LOADER_OFF) count=$(LOADER_CNTS) conv=notrunc
	$(DD) if=$(SETUP_BIN) of=$(FLOPPYA_IMG) bs=512 seek=$(SETUP_OFF) count=$(SETUP_CNTS) conv=notrunc
	$(DD) if=$(KERNEL_ELF) of=$(FLOPPYA_IMG) bs=512 seek=$(KERNEL_OFF) count=$(KERNEL_CNTS) conv=notrunc
	$(DD) if=$(FILESRV_BIN) of=$(FLOPPYA_IMG) bs=512 seek=$(FILESRV_OFF) count=$(FILESRV_CNTS) conv=notrunc
	$(FATFS_BIN) $(HDA_IMG) $(ROM_DIR) $(ROM_DISK_SZ)

#$(DD) if=$(INITSRV_BIN) of=$(HDA_IMG) bs=512 seek=200 count=200 conv=notrunc


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
	$(TRUNC) -s $(FLOPPYA_SZ) $(FLOPPYA_IMG)
	$(TRUNC) -s $(HDA_SZ) $(HDA_IMG)
	$(TRUNC) -s $(HDB_SZ) $(HDB_IMG) 
	$(MAKE) -s -C  $(FATFS_DIR)
	$(MAKE) -s -C  $(LIBRARY_DIR)
	$(MAKE) -s -C  $(SERVICE_DIR)
	$(MAKE) -s -C  $(USER_DIR)
	$(FATFS_BIN) $(HDB_IMG) $(ROM_DIR) $(ROM_DISK_SZ)

# 清理环境。
debuild: 
	$(MAKE) -s -C  $(KERNSRC) clean
	$(MAKE) -s -C  $(FATFS_DIR) clean
	$(MAKE) -s -C  $(LIBRARY_DIR) clean
	$(MAKE) -s -C  $(SERVICE_DIR) clean
	$(MAKE) -s -C  $(USER_DIR) clean
	-$(RM) -r $(ROM_DIR)/bin
	-$(RM) -r $(ROM_DIR)/sbin
	-$(RM) -r $(IMAGE_DIR)
	
# 写入rom
rom: 
	$(FATFS_BIN) $(HDB_IMG) $(ROM_DIR) $(ROM_DISK_SZ)

# 重新编译所有库
lib: 
	$(MAKE) -s -C  $(LIBRARY_DIR)

lib_c: 
	$(MAKE) -s -C  $(LIBRARY_DIR) clean
	
# 重新编译所有服务
srv: 
	$(MAKE) -s -C  $(SERVICE_DIR)

srv_c: 
	$(MAKE) -s -C  $(SERVICE_DIR) clean

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
#	1. IDE DISK：-hda $(HDA_IMG) -hdb $(HDB_IMG)
# 	2. AHCI DISK: -drive id=disk0,file=$(HDA_IMG),if=none \
		-drive id=disk1,file=$(HDB_IMG),if=none \
		-device ahci,id=ahci \
		-device ide-drive,drive=disk0,bus=ahci.0 \
		-device ide-drive,drive=disk1,bus=ahci.1 \


QEMU_ARGUMENT = -m 256M \
		-name "XBOOK Development Platform for x86" \
		-fda $(FLOPPYA_IMG) \
		-drive id=disk0,file=$(HDA_IMG),if=none \
		-drive id=disk1,file=$(HDB_IMG),if=none \
		-device ahci,id=ahci \
		-device ide-drive,drive=disk0,bus=ahci.0 \
		-device ide-drive,drive=disk1,bus=ahci.1 \
		-boot a \
		-serial stdio

#		-fda $(FLOPPYA_IMG) -hda $(HDA_IMG) -hdb $(HDB_IMG) -boot a \
#		-net nic,model=rtl8139 -net tap,ifname=tap0,script=no,downscript=no 

# qemu启动
qemu: all
	$(QEMU) $(QEMU_ARGUMENT)

# 调试配置：-S -gdb tcp::10001,ipv4
qemudbg: all
	$(QEMU) -S -gdb tcp::10001,ipv4 $(QEMU_ARGUMENT)
