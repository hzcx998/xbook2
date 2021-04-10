# MIT License
# Copyright (c) 2020 Jason Hu, Zhu Yu
sinclude ../scripts/localenv.mk

X_PREPARE	+= $(obj)/boot.bin $(obj)/loader.bin
X_CLEAN		:= $(obj)/boot.bin $(obj)/loader.bin $(obj)/setup.elf 

$(obj)/boot.bin: $(src)/boot.asm
	@echo [BOOT.BIN] $@
	@$(AS) -f bin -I $(obj)/ -o $@ $<

$(obj)/loader.bin: $(src)/loader.asm
	@echo [LOADER.BIN] $@
	@$(AS)  -f bin -I $(obj)/ -o $@ $<

X_ASFLAGS	:= $(ENV_AFLAGS) -I $(obj)/
X_CFLAGS    := $(ENV_CFLAGS)

X_INCDIRS	:=
X_LDFLAGS	:= $(ENV_LDFLAGS)
AS			:=	$(ENV_AS)
LD			:= 	$(ENV_LD)

LD_SCRITP	:= $(ARCH)/boot/setup.ld

export AS LD

SRC			+= start.asm setup.c lib.c page.c
NAME		:= setup.bin

define CUSTOM_TARGET_CMD
echo [SETUP.BIN] $@; \
$(LD) $(X_LDFLAGS) -n -T $(LD_SCRITP) -o $(obj)/setup.elf $(X_OBJS); \
$(OC) -R .note -R .comment -S -O binary $(obj)/setup.elf $@
endef 