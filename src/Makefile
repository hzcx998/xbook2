sinclude ../scripts/env.mk

ifeq ("$(origin G)", "command line")
ifeq ($(G),on)
X_CFLAGS	+= -O0 -g -ggdb
endif
endif

ARCH		:= arch/x86

X_ASFLAGS	+= -f elf -I $(ARCH)/include/
X_CFLAGS    += -fno-builtin -Wall -Wunused -fno-PIE -m32 -fno-stack-protector -O2 -std=gnu99 -fno-strict-aliasing
X_INCDIRS	+= include/ $(ARCH)/include/ net/lwip/include/ net/lwip/include/ipv4/
X_LDFLAGS	+= -no-pie -e kernel_start -Ttext 0x80100000

AS			:=	nasm
LD			:=  ld -m elf_i386
export AS LD

SRC			+= $(ARCH)/ init/ ipc/ task/ vmm/ lib/ kernel/ fs/ net/ gui/

NAME		:= kernel.elf

MODULE		+= $(ARCH)/boot

define CUSTOM_TARGET_CMD
echo [KERNEL] $@; \
$(LD) $(X_LDFLAGS) -n -T $(ARCH)/kernel.ld -o $@ $(X_OBJS)
endef