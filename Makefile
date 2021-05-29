# MIT License
# Copyright (c) 2020 Jason Hu, Zhu Yu
sinclude scripts/localenv.mk

# tools
MAKE		= make
FATFS_DIR	= tools/fatfs

# System environment variable.
ifeq ($(OS),Windows_NT)
	FATFS_BIN		:= fatfs
else
	FATFS_BIN		:= $(FATFS_DIR)/fatfs
endif

# host tools
TRUNC		= truncate
RM			= rm
DD			= dd
MKDIR		= mkdir
CP			= cp

# arch tools
OBJDUMP		= $(CROSS_COMPILE)objdump
OBJCOPY		= $(CROSS_COMPILE)objcopy
GDB			= $(CROSS_COMPILE)gdb

# virtual machine
QEMUPREFIX	:= qemu-system-

# rom
ROM_DIR		= develop/rom

IMAGE_DIR	= develop/image

# environment dir

LIBS_DIR	= libs
SBIN_DIR	= sbin
BIN_DIR		= bin

# arch dir
KERNSRC		= ./src
ARCH		= $(KERNSRC)/arch/$(ENV_ARCH)

MOUNT_DIR	:= /mnt

# 管理员执行
ifeq ($(OS),Windows_NT)
	SUDO		:= 
	USE_FATFS	:= yes
else
	SUDO		:= sudo
# use fatfs tools for making image
	USE_FATFS	:= no
endif

ifeq ($(ENV_ARCH),x86) # x86-i386
ifeq ($(ENV_MACH),mach-i386) # x86-i386
	QEMU 		:= $(QEMUPREFIX)i386
	# kernel boot binary
	BOOT_BIN 	= $(ARCH)/$(ENV_MACH)/boot/boot.bin
	LOADER_BIN 	= $(ARCH)/$(ENV_MACH)/boot/loader.bin
	SETUP_BIN 	= $(ARCH)/$(ENV_MACH)/boot/setup.bin

	# images 
	FLOPPYA_IMG	= $(IMAGE_DIR)/a.img
	HDA_IMG		= $(IMAGE_DIR)/c.img
	HDB_IMG		= $(IMAGE_DIR)/d.img
	BOOT_DISK	= $(FLOPPYA_IMG)
	FS_DISK		= $(HDB_IMG)

	# image size
	FLOPPYA_SZ	= 1474560  # 1.44 MB
	HDA_SZ		= 33554432 # 32 MB
	HDB_SZ		= 134217728 # 128 M

	#kernel disk offset
	LOADER_OFF 	= 2
	LOADER_CNTS = 8

	SETUP_OFF 	= 10
	SETUP_CNTS 	= 90

	KERNEL_OFF 	= 100
	KERNEL_CNTS	= 1024		# assume 512kb 

else

endif # ($(ENV_MACH),mach-i386)
else ifeq ($(ENV_ARCH),riscv64) # riscv64
	SD			?= /dev/sdb
ifeq ($(ENV_MACH),mach-qemu) # riscv64 qemu
	QEMU 		:= $(QEMUPREFIX)riscv64
	RUSTSBI 	= $(ARCH)/$(ENV_MACH)/boot/SBI/sbi-qemu

	HDA_SZ		= 134217728 # 128 M

	HDA_IMG		= $(IMAGE_DIR)/c.img
	FS_DISK		= $(HDA_IMG)
else ifeq ($(ENV_MACH),mach-k210) # riscv64 k210
	RUSTSBI 	= $(ARCH)/$(ENV_MACH)/boot/SBI/sbi-k210

	HDA_SZ		= 134217728 # 128 M

	HDA_IMG		= $(IMAGE_DIR)/c.img
	FS_DISK		= $(HDA_IMG)

ifeq ($(ENV_REMOTE_TEST),yes)
	K210_BIN 	= ./k210.bin
else
	K210_BIN 	= $(KERNSRC)/k210.bin
endif
	K210_ASM 	= $(KERNSRC)/k210.asm.dump
	K210_SERIALPORT := /dev/ttyUSB0

endif # ($(ENV_MACH),mach-qemu)
endif # ($(ENV_ARCH),riscv64)

# kernel file
KERNEL_ELF 	= $(KERNSRC)/kernel.elf
KERNEL_BIN 	= $(KERNSRC)/kernel.bin

DUMP_FILE	?= $(KERNEL_ELF)
DUMP_FLAG	?= 

$(warning $(ENV_MACH))
$(warning $(ENV_ARCH))

# 参数
.PHONY: all kernel build debuild qemu qemudbg user user_clean dump k210bin

# remote test for k210
ifeq ($(ENV_REMOTE_TEST),yes)
all: kernimg
	$(OBJCOPY) $(KERNEL_ELF) --strip-all -O binary $(KERNEL_BIN)
	$(OBJCOPY) $(RUSTSBI) --strip-all -O binary $(K210_BIN)
	$(DD) if=$(KERNEL_BIN) of=$(K210_BIN) bs=128k seek=1
else
all : kernimg
ifeq ($(USE_FATFS),yes)
	$(FATFS_BIN) $(FS_DISK) $(ROM_DIR) 0
else
	-@$(SUDO) umount $(MOUNT_DIR)
	@$(SUDO) mount $(FS_DISK) $(MOUNT_DIR)
	@$(SUDO) $(CP) -r $(ROM_DIR)/* $(MOUNT_DIR)
	@$(SUDO) umount $(MOUNT_DIR)
endif
endif

ifeq ($(ENV_ARCH),x86) # x86-i386
# run启动虚拟机
run: qemu
else ifeq ($(ENV_ARCH),riscv64) # riscv64
ifeq ($(ENV_MACH),mach-qemu) # riscv64 qemu
# run启动虚拟机
run: qemu
else ifeq ($(ENV_MACH),mach-k210) # riscv64 k210
# 写入串口启动
run: all
	$(OBJCOPY) $(KERNEL_ELF) --strip-all -O binary $(KERNEL_BIN)
	$(OBJCOPY) $(RUSTSBI) --strip-all -O binary $(K210_BIN)
	$(DD) if=$(KERNEL_BIN) of=$(K210_BIN) bs=128k seek=1
	$(OBJDUMP) -D -b binary -m riscv $(K210_BIN) > $(K210_ASM)
	@echo "K210 run..."
ifeq ($(OS),Windows_NT)
#@python3 ./tools/kflash.py -p $(K210_SERIALPORT) -b 1500000 -t $(K210_BIN)
else
	@$(SUDO) chmod 777 $(K210_SERIALPORT)
	@python3 ./tools/kflash.py -p $(K210_SERIALPORT) -b 1500000 -t $(K210_BIN)
endif

endif
endif

kernel:
	@$(MAKE) -s -C  $(KERNSRC)

clean:
	@$(MAKE) -s -C $(KERNSRC) clean
ifeq ($(ENV_REMOTE_TEST),yes)
	-$(RM) $(K210_BIN)
endif

kernimg: kernel
ifeq ($(ENV_ARCH),x86)
ifeq ($(ENV_MACH),mach-i386)
	$(DD) if=$(BOOT_BIN) of=$(BOOT_DISK) bs=512 count=1 conv=notrunc
	$(DD) if=$(LOADER_BIN) of=$(BOOT_DISK) bs=512 seek=$(LOADER_OFF) count=$(LOADER_CNTS) conv=notrunc
	$(DD) if=$(SETUP_BIN) of=$(BOOT_DISK) bs=512 seek=$(SETUP_OFF) count=$(SETUP_CNTS) conv=notrunc
	$(DD) if=$(KERNEL_ELF) of=$(BOOT_DISK) bs=512 seek=$(KERNEL_OFF) count=$(KERNEL_CNTS) conv=notrunc
endif # ($(ENV_MACH),mach-i386)
else ifeq ($(ENV_ARCH),riscv64)
ifeq ($(ENV_MACH),mach-qemu)
# 将rustsbi和内核写入内核镜像

else ifeq ($(ENV_MACH),mach-k210)

endif # ($(ENV_MACH),mach-qemu)
endif # ($(ENV_ARCH),riscv64)

# 构建环境。镜像>工具>环境>rom
build: buildimg
	-$(MKDIR) $(ROM_DIR)/bin
	-$(MKDIR) $(ROM_DIR)/sbin 
ifeq ($(OS),Windows_NT)
else
	$(MAKE) -s -C  $(FATFS_DIR)
endif # ($(OS),Windows_NT)s
ifeq ($(ENV_ARCH),x86)
	$(MAKE) -s -C  $(LIBS_DIR)
	$(MAKE) -s -C  $(SBIN_DIR)
	$(MAKE) -s -C  $(BIN_DIR)
else ifeq ($(ENV_ARCH),riscv64)
	$(MAKE) -s -C  $(LIBS_DIR)
	$(MAKE) -s -C  $(SBIN_DIR)
	$(MAKE) -s -C  $(BIN_DIR)
endif # ($(ENV_ARCH),riscv64)
ifeq ($(USE_FATFS),yes)
	$(FATFS_BIN) $(FS_DISK) $(ROM_DIR) 0
else
	-@$(SUDO) umount $(MOUNT_DIR)
	@$(SUDO) mount $(FS_DISK) $(MOUNT_DIR)
	@$(SUDO) $(CP) -r $(ROM_DIR)/* $(MOUNT_DIR)
	@$(SUDO) umount $(MOUNT_DIR)
endif

buildimg:
	-$(MKDIR) $(IMAGE_DIR)
ifeq ($(ENV_ARCH),x86)
	$(TRUNC) -s $(FLOPPYA_SZ) $(FLOPPYA_IMG)
	$(TRUNC) -s $(HDA_SZ) $(HDA_IMG)
	$(TRUNC) -s $(HDB_SZ) $(HDB_IMG)
else ifeq ($(ENV_ARCH),riscv64)
ifeq ($(USE_FATFS),yes)
	$(TRUNC) -s $(HDA_SZ) $(HDA_IMG) # fs disk
else
	# 构建文件系统命令
	$(DD) if=/dev/zero of=$(HDA_IMG) bs=512 count=6144
	mkfs.vfat -F 32 $(HDA_IMG)
endif # USE_FATFS
endif # ($(ENV_ARCH),x86)

# 清理环境。
debuild: 
	$(MAKE) -s -C  $(KERNSRC) clean
ifeq ($(OS),Windows_NT)
else
	$(MAKE) -s -C  $(FATFS_DIR) clean
endif # ($(OS),Windows_NT)
	$(MAKE) -s -C  $(LIBS_DIR) clean
	$(MAKE) -s -C  $(SBIN_DIR) clean
	$(MAKE) -s -C  $(BIN_DIR) clean
	-$(RM) -r $(ROM_DIR)/bin
	-$(RM) -r $(ROM_DIR)/sbin
	-$(RM) -r $(ROM_DIR)/acct
	-$(RM) -r $(IMAGE_DIR)
	
user: 
ifeq ($(ENV_ARCH),x86)
	$(MAKE) -s -C  $(LIBS_DIR) && \
	$(MAKE) -s -C  $(SBIN_DIR) && \
	$(MAKE) -s -C  $(BIN_DIR)
else ifeq ($(ENV_ARCH),riscv64)
	$(MAKE) -s -C  $(LIBS_DIR) && \
	$(MAKE) -s -C  $(SBIN_DIR) && \
	$(MAKE) -s -C  $(BIN_DIR)
endif # ($(ENV_ARCH),x86)

user_clean: 
	$(MAKE) -s -C  $(LIBS_DIR) clean && \
	$(MAKE) -s -C  $(SBIN_DIR) clean && \
	$(MAKE) -s -C  $(BIN_DIR) clean


# Write sdcard
# 1. copy data to disk
# 2. copy fs.img to disk
sdcard: build
	@if [ "$(SD)" != "" ]; then \
		echo "flashing into sd card..."; \
		$(SUDO) $(DD) if=$(FS_DISK) of=$(SD); \
	else \
		echo "sd card not detected!"; fi

dump:
ifeq ($(ENV_ARCH),x86)
ifeq ($(ENV_MACH),mach-i386)
	$(OBJDUMP) -M intel -D $(DUMP_FLAG) $(DUMP_FILE) > $(DUMP_FILE).dump
endif # ($(ENV_MACH),mach-i386)
else ifeq ($(ENV_ARCH),riscv64)
	$(OBJDUMP) -D $(DUMP_FLAG) $(DUMP_FILE) > $(DUMP_FILE).dump
endif # ($(ENV_ARCH),x86)
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
               
ifeq ($(OS),Windows_NT)
QEMU_KVM := -accel hax
else
QEMU_KVM := -enable-kvm
endif
QEMU_KVM := # no virutal

QEMU_ARGUMENT	:= 	-name "Xbook2 Development Platform for $(ENV_ARCH)-$(ENV_MACH)"
QEMU_ARGUMENT	+= 	$(QEMU_KVM)
# Disk config
ifeq ($(ENV_ARCH),x86)
QEMU_ARGUMENT	+= 	-m 512m 
QEMU_ARGUMENT	+= 	-fda $(FLOPPYA_IMG) \
					-drive id=disk0,file=$(HDA_IMG),if=none \
					-drive id=disk1,file=$(HDB_IMG),if=none \
					-device ahci,id=ahci \
					-device ide-drive,drive=disk0,bus=ahci.0 \
					-device ide-drive,drive=disk1,bus=ahci.1 \
					-boot a
QEMU_ARGUMENT	+= 	-rtc base=localtime \
					-serial stdio
QEMU_ARGUMENT	+= 	-soundhw sb16 \
					-soundhw pcspk
else ifeq ($(ENV_ARCH),riscv64)
# cpus
ifndef CPUS
CPUS := 2
endif # CPUS
QEMU_ARGUMENT	+= 	-m 6M
QEMU_ARGUMENT	+= 	-machine virt
QEMU_ARGUMENT	+= 	-kernel $(KERNEL_ELF)
QEMU_ARGUMENT	+= 	-nographic
QEMU_ARGUMENT	+= 	-bios $(RUSTSBI)
QEMU_ARGUMENT	+= 	-smp $(CPUS)
QEMU_ARGUMENT 	+= 	-drive file=$(FS_DISK),if=none,format=raw,id=x0 
QEMU_ARGUMENT 	+= 	-device virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0
endif # ($(ENV_ARCH),x86)

#		-fda $(FLOPPYA_IMG) -hda $(HDA_IMG) -hdb $(HDB_IMG) -boot a \
#		-net nic,model=rtl8139 -net tap,ifname=tap0,script=no,downscript=no \

# qemu启动
qemu: all
	$(QEMU) $(QEMU_ARGUMENT)


# 调试配置：-S -gdb tcp::10001,ipv4
qemudbg:
	$(QEMU) -S -gdb tcp::10001,ipv4 $(QEMU_ARGUMENT)

# 连接gdb server: target remote localhost:10001
gdb:
	$(GDB) $(KERNEL_ELF)