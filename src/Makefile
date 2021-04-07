sinclude ../scripts/env.mk

ifeq ("$(origin G)", "command line")
ifeq ($(G),on)
X_CFLAGS	+= -O0 -g -ggdb
endif
endif

ARCH		:= arch/x86

X_ASFLAGS	+= -f elf -I $(ARCH)/include/
X_CFLAGS    += -fno-builtin -Wall -Wunused -fno-PIE -m32 -fno-stack-protector -std=gnu99 -O3 -fno-strict-aliasing
X_INCDIRS	+= include/ $(ARCH)/include/ 
X_LDFLAGS	+= -no-pie -e kernel_start -Ttext 0x80100000

AS			:=	nasm

ifeq ($(HOSTOS),macos)
	LD			:=  i386-elf-ld -m elf_i386
else
	LD			:=  ld -m elf_i386
endif

export AS LD

SRC			+= $(ARCH)/ init/ ipc/ task/ vmm/ lib/ kernel/ fs/ drivers/

NAME		:= kernel.elf

MODULE		+= $(ARCH)/boot

define CUSTOM_TARGET_CMD
echo [KERNEL] $@; \
$(LD) $(X_LDFLAGS) -n -T $(ARCH)/kernel.ld -o $@ $(X_OBJS)
endef