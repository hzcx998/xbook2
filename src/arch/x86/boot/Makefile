ifeq ($(KERN_VBE_MODE),y)
	X_CFLAGS += -DKERN_VBE_MODE
endif
ifeq ($(BOOT_MODE),$(BOOT_GRUB2_MODE))
	SRC	+= multiboot2/
endif