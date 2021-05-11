ifneq ($(BOOT_MODE),$(BOOT_GRUB2_MODE))
	X_CFLAGS += -D$(BOOT_GRUB2_MODE)
endif

SRC	+= entry.asm
SRC	+= arch.c
SRC	+= debug.c
SRC	+= cpu/*.c cpu/*.asm
SRC	+= hardware/*.c
SRC	+= interrupt/*.c interrupt/*.asm
SRC	+= mm/*.c
SRC	+= task/*.c task/*.asm
SRC	+= bus/*.c
SRC	+= drivers/