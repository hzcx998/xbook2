sinclude ../scripts/env.mk
sinclude ../scripts/localenv.mk

ifeq ("$(origin G)", "command line")
ifeq ($(G),on)
X_CFLAGS	+= -O0 -g -ggdb
endif
endif

ARCH_DIR		:= arch/$(ENV_ARCH)

X_ASFLAGS	:= $(ENV_AFLAGS) -I $(ARCH_DIR)/include/
X_CFLAGS    := $(ENV_CFLAGS)

#X_INCDIRS	:= include/ $(ARCH_DIR)/include/ $(ARCH_DIR)/$(ENV_MACH)/include

ifeq ($(ENV_MACH), mach-i386)
X_INCDIRS	:= include/ $(ARCH_DIR)/include/ $(ARCH_DIR)/$(ENV_MACH)/include
else
# 进行特殊化处理
X_INCDIRS	:= include/ $(ARCH_DIR)/include/ $(ARCH_DIR)/mach-qemu/include
endif
X_LDFLAGS	:=  $(ENV_LDFLAGS)

AS			:=	$(ENV_AS)
LD			:=  $(ENV_LD)

export AS LD

SRC			:= $(ARCH_DIR)/ init/ ipc/ task/ vmm/ lib/ kernel/ fs/
ifeq ($(ENV_MACH), mach-i386)
SRC			+= drivers/ net/
endif

NAME		:= kernel.elf

# sub module
ifeq ($(ENV_MACH), mach-i386)
MODULE		+= $(ARCH_DIR)/$(ENV_MACH)/boot
endif

define CUSTOM_TARGET_CMD
echo [KERNEL] $@; \
$(LD) $(X_LDFLAGS) -n -T $(ARCH_DIR)/$(ENV_MACH)/kernel.ld -o $@ $(X_OBJS)
endef