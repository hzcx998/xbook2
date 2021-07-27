# MIT License
# Copyright (c) 2020 Jason Hu, Zhu Yu

BUILD_DIR   = ../../src
OS_NAME    ?= XBook
OS_ISO_NAME = $(OS_NAME).iso

#MKISOFS    = mkisofs
MKISOFS     = genisoimage

BOOT_DIR      = boot
GRUB_DIR      = $(BOOT_DIR)/grub
GRUB_ELTORITO = $(BOOT_DIR)/stage2_eltorito
MENU_CONF     = $(GRUB_DIR)/kernels.cfg
EFI_CONF      = ./compile.cfg
EFI32_FILE    = ./efi/boot/bootia32.efi
EFI64_FILE    = ./efi/boot/bootx64.efi
ISO_FILE_NAME = $(BUILD_DIR)/$(OS_ISO_NAME)
ISO_DIR       = ./iso
EFI_BOOT      = efi.img
INITRD_PATH   = $(ISO_DIR)/$(BOOT_DIR)
INITRD_IMG    = initrd.img
INITRD_CONF   = initrd.cfg

X86_GRUB_MODULES = \
	affs afs all_video bitmap bitmap_scale elf eval ntfs \
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

all:
	rm -rf ./efi/boot
	mkdir -p ./efi/boot
	grub-mkimage \
		-d $(GRUB_DIR)/i386-efi \
		-c $(EFI_CONF) \
		-p $(GRUB_DIR) \
		-o $(EFI32_FILE) \
		-O i386-efi \
		$(X86_GRUB_MODULES)
	grub-mkimage \
		-d $(GRUB_DIR)/x86_64-efi \
		-c $(EFI_CONF) \
		-p $(GRUB_DIR) \
		-o $(EFI64_FILE) \
		-O x86_64-efi \
		$(X86_GRUB_MODULES)
	rm -f -r $(ISO_DIR)
	mkdir -p $(ISO_DIR)
	dd if=/dev/zero of=$(ISO_DIR)/$(EFI_BOOT) bs=512 count=8192
	mkfs.msdos -F 12 $(ISO_DIR)/$(EFI_BOOT)
	mmd -i $(ISO_DIR)/$(EFI_BOOT) ::efi
	mmd -i $(ISO_DIR)/$(EFI_BOOT) ::efi/boot
	mcopy -i $(ISO_DIR)/$(EFI_BOOT) $(EFI32_FILE) ::$(EFI32_FILE)
	mcopy -i $(ISO_DIR)/$(EFI_BOOT) $(EFI64_FILE) ::$(EFI64_FILE)
	echo "multiboot2 /$(BOOT_DIR)/$(KERNEL)" > $(MENU_CONF)
	echo "module2 /$(BOOT_DIR)/$(INITRD_IMG) 'initrd'" >> $(MENU_CONF)
	echo "boot" >> $(MENU_CONF)
	cp -r ./efi $(ISO_DIR)/efi
	cp -r ./boot $(ISO_DIR)/boot
	mkdir -p $(ISO_DIR)/$(OS_NAME)
	cp $(BUILD_DIR)/$(KERNEL) $(ISO_DIR)/$(BOOT_DIR)/$(KERNEL)
	cd $(BUILD_DIR)/../develop/rom/ && \
  	sed 's/\r//g' ../$(INITRD_CONF) | cpio -o -H newc > ../../tools/grub-2.04/$(INITRD_PATH)/$(INITRD_IMG)
	$(MKISOFS) \
		-graft-points \
		-input-charset utf8 \
		-sysid "" \
		-appid "" \
		-volid "$(OS_ISO_NAME)" \
		-R \
		-no-emul-boot \
		-boot-load-size 4 \
		-boot-info-table -b $(GRUB_ELTORITO) \
		-eltorito-alt-boot -b $(EFI_BOOT) \
		-no-emul-boot \
		-o $(ISO_FILE_NAME) $(ISO_DIR)
	rm -f -r $(ISO_DIR)

clean:
	rm -f $(ISO_FILE_NAME)
	rm -f $(EFI32_FILE)
	rm -f $(EFI64_FILE)