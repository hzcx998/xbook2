# MIT License
# Copyright (c) 2020 Jason Hu, Zhu Yu
all:

# tools
MAKE		= make
TOOL_DIR	= tools
FATFS_DIR	= $(TOOL_DIR)/fatfs
GRUB_DIR	= $(TOOL_DIR)/grub-2.04
BIOS_FW_DIR	= $(TOOL_DIR)/bios_fw

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
OBJDUMP		= objdump
GDB			= gdb

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
FLOPPYA_SZ	= 1474560 # 1.44 MB
HDA_SZ		= 33554432 # 32 MB
HDB_SZ		= 134217728 # 128 M

# environment dir

LIBS_DIR	= libs
SBIN_DIR	= sbin
BIN_DIR		= bin

#kernel disk
LOADER_OFF 	= 2
LOADER_CNTS = 8

SETUP_OFF	= 10
SETUP_CNTS	= 90

KERNEL_OFF	= 100
KERNEL_CNTS	= 1024		# assume 512kb

# arch dir

KERNSRC		= ./src
ARCH		= $(KERNSRC)/arch/x86

# kernel file
KERNEL_ELF	= $(KERNSRC)/kernel.elf

# OS Name
OS_NAME = XBook

# boot mode
export BOOT_GRUB2_MODE = GRUB2
export BOOT_LEGACY_MODE = LEGACY

# legacy boot mode binary
BOOT_BIN	= $(ARCH)/boot/myboot/boot.bin
LOADER_BIN	= $(ARCH)/boot/myboot/loader.bin
SETUP_BIN	= $(ARCH)/boot/myboot/setup.bin

# set default boot mode
export BOOT_MODE ?= $(BOOT_GRUB2_MODE)

# is efi mode? (y/n)
EFI_BOOT_MODE ?= n
# is qemu fat fs? (y/n)
QEMU_FAT_FS ?= n

# has net module? (y/n)
KERN_MODULE_NET	?= n
export KERN_MODULE_NET

# has dwin module? (y/n)
KERN_MODULE_DWIN	?= n
export KERN_MODULE_DWIN

# netcard name: rtl8139/pcnet/e1000
QEMU_NETCARD_NAME	?=pcnet

# netcard type: tap/user
QEMU_NET_MODE ?=user

# is livecd mode? (y/n)
KERN_LIVECD_MODE ?= n
export KERN_LIVECD_MODE

# is vbe mode? (y/n)
KERN_VBE_MODE ?= y
export KERN_VBE_MODE

# qemu config sound? (y/n)
QEMU_SOUND ?= n

DUMP_FILE	?= $(KERNEL_ELF)
DUMP_FLAGS	?= 

# 参数
.PHONY: all kernel build debuild qemu qemudbg user user_clean dump

# 默认所有动作，编译内核后，把引导、内核、init服务、文件服务和rom文件写入磁盘
all : kernel
ifeq ($(BOOT_MODE),$(BOOT_LEGACY_MODE))
	$(DD) if=$(BOOT_BIN) of=$(BOOT_DISK) bs=512 count=1 conv=notrunc
	$(DD) if=$(LOADER_BIN) of=$(BOOT_DISK) bs=512 seek=$(LOADER_OFF) count=$(LOADER_CNTS) conv=notrunc
	$(DD) if=$(SETUP_BIN) of=$(BOOT_DISK) bs=512 seek=$(SETUP_OFF) count=$(SETUP_CNTS) conv=notrunc
	$(DD) if=$(KERNEL_ELF) of=$(BOOT_DISK) bs=512 seek=$(KERNEL_OFF) count=$(KERNEL_CNTS) conv=notrunc
else
ifeq ($(BOOT_MODE),$(BOOT_GRUB2_MODE))
	@$(MAKE) -s -C $(GRUB_DIR) KERNEL=$(subst $(KERNSRC)/,,$(KERNEL_ELF)) OS_NAME=$(OS_NAME)
endif
endif
ifeq ($(QEMU_FAT_FS),n)
	$(FATFS_BIN) $(FS_DISK) $(ROM_DIR) 0
endif

# run启动虚拟机
run: qemu

# 先写rom，在编译内核
kernel:
	@$(MAKE) -s -C $(KERNSRC)

clean:
	@$(MAKE) -s -C $(KERNSRC) clean

# 构建环境。镜像>工具>环境>rom
build:
	-$(MKDIR) $(IMAGE_DIR)
	-$(MKDIR) $(ROM_DIR)/bin
	-$(MKDIR) $(ROM_DIR)/sbin
	$(TRUNC) -s $(FLOPPYA_SZ) $(FLOPPYA_IMG)
	$(TRUNC) -s $(HDA_SZ) $(HDA_IMG)
	$(TRUNC) -s $(HDB_SZ) $(HDB_IMG)
ifeq ($(OS),Windows_NT)
else
	$(MAKE) -s -C $(FATFS_DIR)
endif
	$(MAKE) -s -C $(LIBS_DIR)
	$(MAKE) -s -C $(SBIN_DIR)
	$(MAKE) -s -C $(BIN_DIR)
ifeq ($(QEMU_FAT_FS),n)
	$(FATFS_BIN) $(FS_DISK) $(ROM_DIR) 0
endif

# 清理环境。
debuild:
	$(MAKE) -s -C $(KERNSRC) clean
ifeq ($(OS),Windows_NT)
else
	$(MAKE) -s -C $(FATFS_DIR) clean
endif
	$(MAKE) -s -C $(LIBS_DIR) clean
	$(MAKE) -s -C $(SBIN_DIR) clean
	$(MAKE) -s -C $(BIN_DIR) clean
	$(MAKE) -s -C $(GRUB_DIR) clean
	-$(RM) -r $(ROM_DIR)/bin
	-$(RM) -r $(ROM_DIR)/sbin
	-$(RM) -r $(ROM_DIR)/acct
	-$(RM) -r $(IMAGE_DIR)

user:
	$(MAKE) -s -C $(LIBS_DIR) && \
	$(MAKE) -s -C $(SBIN_DIR) && \
	$(MAKE) -s -C $(BIN_DIR)

user_clean:
	$(MAKE) -s -C $(LIBS_DIR) clean && \
	$(MAKE) -s -C $(SBIN_DIR) clean && \
	$(MAKE) -s -C $(BIN_DIR) clean

dump:
	$(OBJDUMP) $(DUMP_FLAGS) -M intel -D $(DUMP_FILE) > $(DUMP_FILE).dump

#-hda $(HDA_IMG) -hdb $(HDB_IMG)
# 网卡配置:
#	-net nic,vlan=0,model=rtl8139,macaddr=12:34:56:78:9a:be
# 网络模式：
#	1.User mode network(Slirp) :User网络
#		-net user
#	2.Tap/tun network : Tap网络
#		-net tap
#		-net tap,vlan=0,ifname=tap0
#	example: -net nic,model=rtl8139 -net tap,ifname=tap0,script=no,downscript=no

# 音频配置：
#	a.使用AC97卡： -device AC97
#	b.使用声霸卡： -device sb16
#	c.使用HDA卡： -device intel-hda -device hda-duplex
# 控制台串口调试： -serial stdio

# 磁盘配置：
#	1. IDE DISK：-hda $(HDA_IMG) -hdb $(HDB_IMG) \
#	2. AHCI DISK: -drive id=disk0,file=$(HDA_IMG),if=none \
		-drive id=disk1,file=$(HDB_IMG),if=none \
		-device ahci,id=ahci \
		-device ide-drive,drive=disk0,bus=ahci.0 \
		-device ide-drive,drive=disk1,bus=ahci.1 \

ifeq ($(OS),Windows_NT)
QEMU_KVM := -accel hax
else
QEMU_KVM := -enable-kvm
endif
QEMU_KVM := # no virutal

ifeq ($(QEMU_FAT_FS),y)
	HDB_IMG :=fat:rw:./develop/rom
endif

QEMU_ARGUMENT := -m 512m $(QEMU_KVM) \
		-name "XBOOK Development Platform for x86" \
		-rtc base=localtime \
		-boot a \
		-serial stdio

ifeq ($(QEMU_SOUND),y)
QEMU_ARGUMENT += -device sb16 \
		-device AC97 \
		-device intel-hda -device hda-duplex
endif

DISK_AHCI = y
ifeq ($(KERN_LIVECD_MODE),n)
ifeq ($(DISK_AHCI),y)
QEMU_ARGUMENT += -drive id=disk0,file=$(HDA_IMG),format=raw,if=none \
		-drive id=disk1,file=$(HDB_IMG),format=raw,if=none \
		-device ahci,id=ahci \
		-device ide-hd,drive=disk0,bus=ahci.0 \
		-device ide-hd,drive=disk1,bus=ahci.1
else
QEMU_ARGUMENT += -hda $(HDA_IMG) -hdb $(HDB_IMG)
endif # DISK_AHCI
endif # KERN_LIVECD_MODE

ifeq ($(KERN_MODULE_NET),y)
	QEMU_ARGUMENT += -net nic,model=$(QEMU_NETCARD_NAME)

ifeq ($(QEMU_NET_MODE),tap)
	QEMU_ARGUMENT += -net tap,ifname=tap0,script=no,downscript=no 
else
	QEMU_ARGUMENT += -net user
endif

endif

ifeq ($(BOOT_MODE),$(BOOT_LEGACY_MODE))
QEMU_ARGUMENT += -drive file=$(FLOPPYA_IMG),format=raw,index=0,if=floppy
endif

#		-fda $(FLOPPYA_IMG) -hda $(HDA_IMG) -hdb $(HDB_IMG) -boot a \
#		-net nic,model=rtl8139 -net tap,ifname=tap0,script=no,downscript=no \

# qemu启动
qemu: all
ifeq ($(BOOT_MODE),$(BOOT_LEGACY_MODE))
	$(QEMU) $(QEMU_ARGUMENT)
else
ifeq ($(BOOT_MODE),$(BOOT_GRUB2_MODE))
ifeq ($(EFI_BOOT_MODE),n)
	$(QEMU) $(QEMU_ARGUMENT) -cdrom $(KERNSRC)/$(OS_NAME).iso
else
	$(QEMU) $(QEMU_ARGUMENT) -bios $(BIOS_FW_DIR)/IA32_OVMF.fd -cdrom $(KERNSRC)/$(OS_NAME).iso
endif
endif
endif

QEMU_GDB_OPT	:= -S -gdb tcp::10001,ipv4

# 调试配置：-S -gdb tcp::10001,ipv4
qemudbg:
ifeq ($(BOOT_MODE),$(BOOT_LEGACY_MODE))
	$(QEMU) $(QEMU_GDB_OPT) $(QEMU_ARGUMENT)
else
ifeq ($(BOOT_MODE),$(BOOT_GRUB2_MODE))
ifeq ($(EFI_BOOT_MODE),n)
	$(QEMU) $(QEMU_GDB_OPT) $(QEMU_ARGUMENT) -cdrom $(KERNSRC)/$(OS_NAME).iso
else
	$(QEMU) $(QEMU_GDB_OPT) $(QEMU_ARGUMENT) -bios $(BIOS_FW_DIR)/IA32_OVMF.fd -cdrom $(KERNSRC)/$(OS_NAME).iso
endif
endif
endif

# 连接gdb server: target remote localhost:10001
gdb:
	$(GDB) $(KERNEL_ELF)