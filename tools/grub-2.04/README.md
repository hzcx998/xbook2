### 文件说明
```
├─boot
│  │─grub
│  │   ├─i386-efi
│  │   ├─i386-pc
│  │   ├─locales
│  │   │─themes
│  │   │   └─XBook-live
│  │   │       └─icons
│  │   ├─grub.cfg
│  │   ├─kernels.cfg
│  │   ├─unicode.pf2
│  │   └─variable.cfg
│  └─stage2_eltorito
│─efi
│   └─boot
├─compile.cfg
├─Makefile
└─OVMF.fd
```
* `efi/boot/` 为efi启动时默认搜索efi的地方，里面将存放编译好的 `bootia32.efi`。
* `boot/` 为对应平台的驱动目录
* `boot/stage2_eltorito` 为grub启动光盘的必要文件。在`boot/grub/i386-pc`中同样存在该文件，但启动速度较慢。
* `boot/grub/i386-pc` 为x86架构传统BIOS启动的基本模块
* `boot/grub/i386-efi` 为x86架构uefi启动的基本模块
* `boot/grub/locales` 语言包
* `boot/themes` 可以存放多个图形界面grub主题，`XBook-live`为默认主题，可在`https://www.gnome-look.org/browse/cat/109/order/latest/`下载其他主题，记得设计对应XBook的logo。
* `boot/grub/grub.cfg` grub启动后配置脚本第二次启动的配置，包括菜单，图形界面，字体等
* `boot/grub/kernels.cfg` XBook的内核启动菜单，由生成ISO文件时自动覆盖内容
* `boot/grub/unicode.pf2` grub图形界面默认字体
* `boot/grub/variable.cfg` grub设定主题配置文件
* `compile.cfg` grub启动后执行的配置脚本
* `Makefile` 用于生成efi、iso的Makefile
* `OVMF.fd` 为ia32的UEFI固件，可在QEMU中`-bios`使用

### Makefile转换说明
建议只改动一下部分变量的`值`
```
# 用于内核kernel.elf文件生成的路径
BUILD_DIR   = ../../src
# ISO中内核路径的文件夹名，同时也是ISO文件的文件名
OS_NAME     = XBook
OS_ISO_NAME = $(OS_NAME).iso
# QEMU测试用的参数，不在此脚本使用QEMU可直接删除
QFLAGS      = -m 256M -name "XBook Grub 2.04 Demo" -rtc base=localtime
# 内核文件全名
KERNEL      = kernel.elf

# 生成ISO的工具，两个工具完全兼容，linux下都可以使用，win下可能只有genisoimage，在new Tools软件包中
#MKISOFS    = mkisofs
MKISOFS     = genisoimage

......

# QEMU测试用ISO通用启动，不在此脚本使用QEMU可直接删除
qemu:
	qemu-system-i386 $(QFLAGS) -soundhw pcspk -cdrom $(ISO_FILE_NAME)

# QEMU测试用ISO，EFI通用启动，不在此脚本使用QEMU可直接删除
qemu-efi:
	qemu-system-i386 $(QFLAGS) -bios ./OVMF.fd -soundhw pcspk -cdrom $(ISO_FILE_NAME)

# 用于清理EFI文件和ISO文件
clean:
	rm $(ISO_FILE_NAME)
	rm $(EFI_FILE)
```

### Makefile指令说明
```
BOOT_DIR      = boot
GRUB_DIR      = $(BOOT_DIR)/grub
GRUB_ELTORITO = $(BOOT_DIR)/stage2_eltorito
MENU_CONF     = $(GRUB_DIR)/kernels.cfg
EFI_CONF      = ./compile.cfg
EFI_FILE      = ./efi/boot/bootia32.efi
ISO_FILE_NAME = $(BUILD_DIR)/$(OS_ISO_NAME)
ISO_TEMP_DIR  = ./iso
EFI_BOOT      = $(BOOT_DIR)/efi.img

.PHONY: efi iso

all: efi iso

efi:
	grub-mkimage \
        # grub模块路径
		-d $(GRUB_DIR)/i386-efi \
        # grub启动后的第一个执行的配置脚本
		-c $(EFI_CONF) \
        # grub根目录
		-p $(GRUB_DIR) \
        # 生成efi/core的路径
		-o $(EFI_FILE) \
        # 启动方式
		-O i386-efi \
        # 需要使用的模块，其实用不了这么多
		affs afs all_video bitmap bitmap_scale elf eval linux \
		blocklist boot btrfs cat chain cmp configfile cpio tar \
		fat file font fshelp gettext gfxmenu date newc png \
		gfxterm gfxterm_background gfxterm_menu gptsync hashsum \
		help hexdump hfs hfsplus hfspluscomp iso9660 jfs jpeg \
		loadenv loopback ls lsacpi datetime disk echo minicmd \
		lsmmap lspci lvm lzopio memdisk multiboot multiboot2 \
		normal part_apple part_bsd part_gpt part_msdos parttool \
		probe procfs random read reboot regexp search halt gzio \
		search_fs_file search_fs_uuid search_label sleep squash4 \
		terminal terminfo test tga time true udf video video_bochs \
		video_cirrus video_colors video_fb videoinfo xzio datehook \
		loadbios appleldr crc efi_gop efi_uga lsefi lsefimmap lsefisystab

iso:
    # 创建虚拟磁盘efi.img
	dd if=/dev/zero of=$(EFI_BOOT) bs=512 count=8192
    # efi.img写入fat12文件系统
	mkfs.msdos -F 12 $(EFI_BOOT)
    # efi.img创建efi文件夹
	mmd -i $(EFI_BOOT) ::efi
    # efi.img创建efi/boot文件夹
	mmd -i $(EFI_BOOT) ::efi/boot
    # 将efi文件拷贝到efi.img对应efi/boot位置
	mcopy -i $(EFI_BOOT) $(EFI_FILE) ::$(EFI_FILE)
    # 动态写入kernels.cfg菜单文件，该指令为multiboot2的方式加载内核
	echo "multiboot2 /$(OS_NAME)/$(KERNEL)" > $(MENU_CONF)
    # 动态写入kernels.cfg菜单文件，该指令启动内核
	echo "boot" >> $(MENU_CONF)
    # 删除之前iso目录（如果iso构建失败可能会残留，导致ISO变更大）
	rm -f -r $(ISO_TEMP_DIR)
    # 创建iso目录
	mkdir -p $(ISO_TEMP_DIR)
    # 复制efi到iso
	cp -r ./efi $(ISO_TEMP_DIR)/efi
    # 复制boot到iso
	cp -r ./boot $(ISO_TEMP_DIR)/boot
    # 在iso创建内核文件夹
	mkdir -p $(ISO_TEMP_DIR)/$(OS_NAME)
    # 拷贝内核文件
	cp $(BUILD_DIR)/$(KERNEL) $(ISO_TEMP_DIR)/$(OS_NAME)/$(KERNEL)
    # 生成ISO文件
	$(MKISOFS) \
        # 转义，作用不大，反正大家都留着就留着
		-graft-points \
        # 编码
		-input-charset utf8 \
        # 指定系统ID
		-sysid "" \
        # 指定描述光盘应用程序Id的文本字符串，可以有128个字符
		-appid "" \
        # 指定要写入主块的卷ID
		-volid "$(OS_ISO_NAME)" \
        # 使用Rock Ridge协议
		-R \
        # 支持El Torito启动
		-no-emul-boot \
        # boot扇区数量
		-boot-load-size 4 \
        # El Torito启动文件
		-boot-info-table -b $(GRUB_ELTORITO) \
        # 从一组新的El Torito启动参数开始。最多63个El Torito引导项可以存储在一张CD上。
		-eltorito-alt-boot -b $(EFI_BOOT) \
        # 指定用于创建El Torito可引导cd的引导映像是“无仿真”映像。系统将在不执行任何磁盘模拟的情况下加载和执行此映像。
		-no-emul-boot \
        -o $(ISO_FILE_NAME) $(ISO_TEMP_DIR)
    # 删除iso
	rm -r $(ISO_TEMP_DIR)
    # 删除efi.img
	rm $(EFI_BOOT)
```

### new Tools说明（全用于WIN）
* `genisoimage` 生成ISO文件，和mkisofs完全兼容，具体参数参考https://www.cnblogs.com/wj78080458/p/9879699.html
* `grub-2.04` grub的模块以及字体、主题和各种安装、写入程序
* `mkfs` 用于虚拟磁盘的文件格式化
* `mtools` 用于虚拟磁盘的文件系统操作
* `rufus-3.13` 用于将ISO刷入U盘启动，Linux下可参考https://askubuntu.com/questions/372607/how-to-create-a-bootable-ubuntu-usb-flash-drive-from-terminal