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
EFI_FILE      = ./efi/boot/bootia32.efi
ISO_FILE_NAME = $(BUILD_DIR)/$(OS_ISO_NAME)
ISO_TEMP_DIR  = ./iso
EFI_BOOT      = $(BOOT_DIR)/efi.img

.PHONY: efi iso

all: efi iso

efi:
	rm -rf ./efi/boot
	mkdir -p ./efi/boot
	grub-mkimage \
		-d $(GRUB_DIR)/i386-efi \
		-c $(EFI_CONF) \
		-p $(GRUB_DIR) \
		-o $(EFI_FILE) \
		-O i386-efi \
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
	dd if=/dev/zero of=$(EFI_BOOT) bs=512 count=8192
	mkfs.msdos -F 12 $(EFI_BOOT)
	mmd -i $(EFI_BOOT) ::efi
	mmd -i $(EFI_BOOT) ::efi/boot
	mcopy -i $(EFI_BOOT) $(EFI_FILE) ::$(EFI_FILE)
	echo "multiboot2 /$(OS_NAME)/$(KERNEL)" > $(MENU_CONF)
	echo "boot" >> $(MENU_CONF)
	rm -f -r $(ISO_TEMP_DIR)
	mkdir -p $(ISO_TEMP_DIR)
	cp -r ./efi $(ISO_TEMP_DIR)/efi
	cp -r ./boot $(ISO_TEMP_DIR)/boot
	mkdir -p $(ISO_TEMP_DIR)/$(OS_NAME)
	cp $(BUILD_DIR)/$(KERNEL) $(ISO_TEMP_DIR)/$(OS_NAME)/$(KERNEL)
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
		-o $(ISO_FILE_NAME) $(ISO_TEMP_DIR)
	rm -f -r $(ISO_TEMP_DIR)
	rm -f $(EFI_BOOT)

clean:
	rm -f $(ISO_FILE_NAME)
	rm -f $(EFI_FILE)