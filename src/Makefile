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

X_INCDIRS	:= include/ $(ARCH_DIR)/include/ $(ARCH_DIR)/$(ENV_MACH)/include

X_LDFLAGS	:=  $(ENV_LDFLAGS)

AS			:=	$(ENV_AS)
LD			:=  $(ENV_LD)

export AS LD

ifeq ($(ENV_MACH), mach-i386)
SRC			:= $(ARCH_DIR)/ init/ ipc/ task/ vmm/ lib/ kernel/ fs/ drivers/ net/
else
SRC			:= $(ARCH_DIR)/ kernel/debug.c lib/string.c lib/vsprintf.c lib/bitmap.c lib/string_obj.c
SRC			+= init/main2.c 
SRC			+= vmm/dma.c vmm/memcache.c vmm/virmem.c vmm/vmm.c vmm/memspace.c vmm/mdl.c vmm/memspace2.c 
SRC			+= kernel/hardirq.c kernel/softirq.c kernel/walltime.c kernel/safety.c 
SRC			+= kernel/clock.c kernel/timer.c kernel/time.c kernel/alarm.c
SRC			+= kernel/tests.c
SRC			+= task/mutexlock.c
SRC			+= task/task.c task/process.c
SRC			+= task/schedule.c
SRC			+= task/semaphore.c
SRC			+= task/exit.c
SRC			+= task/waitqueue.c
SRC			+= task/sleep.c
SRC			+= task/wait.c
SRC			+= task/exec.c

SRC			+= kernel/driver.c
SRC			+= fs/
SRC			+= kernel/account.c kernel/permission.c
SRC			+= lib/sconf.c
SRC			+= lib/ctype.c
SRC			+= ipc/pipe.c
SRC			+= kernel/exception.c
SRC			+= kernel/exit.c
SRC			+= lib/fifobuf.c
SRC			+= lib/fifoio.c
SRC			+= ipc/fifo.c
SRC			+= kernel/initcall.c
SRC			+= kernel/syscall.c
SRC			+= kernel/config.c
SRC			+= drivers/tty/tty.c

#SRC			+= drivers/char/null.c drivers/char/zero.c
#SRC			+= drivers/block/loop.c
#SRC			+= drivers/tty/tty.c drivers/tty/ptty.c

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