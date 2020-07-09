# MIT License
# Copyright (c) 2020 Jason Hu, Zhu Yu

X_PREPARE	+= $(obj)/boot.bin $(obj)/loader.bin
X_CLEAN		:= $(obj)/boot.bin $(obj)/loader.bin $(obj)/setup.elf 

$(obj)/boot.bin: $(src)/boot.asm
	@echo [BOOT.BIN] $@
	@$(AS) -f bin -I $(obj)/ -o $@ $<

$(obj)/loader.bin: $(src)/loader.asm
	@echo [LOADER.BIN] $@
	@$(AS)  -f bin -I $(obj)/ -o $@ $<

X_ASFLAGS	:= -f elf -I $(obj)/
X_CFLAGS    += -fno-builtin -Wall -Wunused -fno-PIE -m32 -fno-stack-protector
X_INCDIRS	:=
X_LDFLAGS	+= -e _start -Ttext 0x91000
AS			:=	nasm
LD			:=  ld -m elf_i386

export AS LD

SRC			+= start.asm setup.c lib.c page.c
NAME		:= setup.bin


define CUSTOM_TARGET_CMD
echo [SETUP.BIN] $@; \
$(LD) $(X_LDFLAGS) -o $(obj)/setup.elf $(X_OBJS); \
$(OC) -R .note -R .comment -S -O binary $(obj)/setup.elf $@
endef