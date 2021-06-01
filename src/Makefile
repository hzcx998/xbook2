sinclude ../scripts/env.mk
sinclude ../scripts/localenv.mk

ARCH		:= arch/x86

X_ASFLAGS	:= $(ENV_AFLAGS) -I $(ARCH)/include/
X_CFLAGS    := $(ENV_CFLAGS) -fno-strict-aliasing

ifeq ("$(origin G)", "command line")
ifeq ($(G),on)
X_CFLAGS	+= -O0 -g -ggdb
X_ASFLAGS	+= -DGDB
endif
endif

X_INCDIRS	:= include/ $(ARCH)/include/

ifeq ($(KERN_MODULE_NET),y)
X_INCDIRS	+= net/lwip/include/ net/lwip/include/ipv4/
X_CFLAGS	+= -DCONFIG_NET
# 如果要在用户态实现网络协议栈，就需要用netremote
# X_CFLAGS	+= -DCONFIG_NETREMOTE
endif

X_LDFLAGS	:=  $(ENV_LDFLAGS)

AS			:=	$(ENV_AS)
LD			:=  $(ENV_LD)

export AS LD
export ARCH

SRC			:= $(ARCH)/ init/ ipc/ task/ vmm/ lib/ kernel/ fs/ drivers/

ifeq ($(KERN_MODULE_NET),y)
SRC			+= net/
endif


NAME		:= kernel.elf

ifeq ($(BOOT_MODE),$(BOOT_LEGACY_MODE))
	KERNEL_LD_FILE = kernel-myboot.ld
	MODULE += $(ARCH)/boot/myboot
else
	KERNEL_LD_FILE = kernel.ld
	MODULE += $(ARCH)/boot
endif

define CUSTOM_TARGET_CMD
echo [KERNEL] $@; \
$(LD) $(X_LDFLAGS) -n -T $(ARCH)/$(KERNEL_LD_FILE) -o $@ $(X_OBJS)
endef