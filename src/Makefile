sinclude ../scripts/env.mk

ifeq ("$(origin G)", "command line")
ifeq ($(G),on)
X_CFLAGS	+= -O0 -g -ggdb
endif
endif

X_ASFLAGS	+= -f elf -I arch/x86/core/
X_CFLAGS    += -fno-builtin -Wall -Wunused -fno-PIE -m32 -fno-stack-protector
X_INCDIRS	+= include/
X_LDFLAGS	+= -no-pie -e kernel_start -Ttext 0x80100000

AS			:=	nasm
LD			:=  ld -m elf_i386
export AS LD

SRC			+= arch/x86/ init/ ipc/ task/ vmm/ driver/ debug/ lib/ kernel/

NAME		:= kernel.elf

MODULE		+= arch/x86/boot

define CUSTOM_TARGET_CMD
echo [KERNEL] $@; \
$(LD) $(X_LDFLAGS) -n -T arch/x86/x86-kernel.ld -o $@ $(X_OBJS)
endef