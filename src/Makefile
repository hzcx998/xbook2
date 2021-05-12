sinclude ../scripts/env.mk
sinclude ../scripts/localenv.mk

ifeq ("$(origin G)", "command line")
ifeq ($(G),on)
X_CFLAGS	+= -O0 -g -ggdb
endif
endif

ARCH		:= arch/x86

X_ASFLAGS	:= $(ENV_AFLAGS) -I $(ARCH)/include/
X_CFLAGS    := $(ENV_CFLAGS) -fno-strict-aliasing

X_INCDIRS	:= include/ $(ARCH)/include/

X_LDFLAGS	:=  $(ENV_LDFLAGS)

AS			:=	$(ENV_AS)
LD			:=  $(ENV_LD)

export AS LD
export ARCH

SRC			:= $(ARCH)/ init/ ipc/ task/ vmm/ lib/ kernel/ fs/ drivers/ net/

NAME		:= kernel.elf

MODULE		+= $(ARCH)/boot

ifeq ($(BOOT_MODE),$(BOOT_LEGACY_MODE))
	KERNEL_LD_FILE = kernel-myboot.ld
	MODULE +=/myboot
else
	KERNEL_LD_FILE = kernel.ld
endif

define CUSTOM_TARGET_CMD
echo [KERNEL] $@; \
$(LD) $(X_LDFLAGS) -n -T $(ARCH)/$(KERNEL_LD_FILE) -o $@ $(X_OBJS)
endef