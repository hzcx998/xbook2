# env var 

#CROSS_COMPILE 	?= 
#PLATFORM		?=
CROSS_COMPILE 	?= riscv-none-embed-
PLATFORM		?= riscv64-qemu

#
# Get platform information about ARCH and MACH from PLATFORM variable.
#
ifeq ($(words $(subst -, , $(PLATFORM))), 2)
ENV_ARCH			:= $(word 1, $(subst -, , $(PLATFORM)))
ENV_MACH			:= mach-$(word 2, $(subst -, , $(PLATFORM)))
else
#ENV_ARCH			:= x86
#ENV_MACH			:= mach-i386
ENV_ARCH			:= riscv64
ENV_MACH			:= mach-qemu
endif

ENV_AFLAGS	:=
CFLAGS	:=
MCFLAGS	:=

# MacOS must need i386-elf-	
ifeq ($(ENV_ARCH), x86)
	ifeq ($(shell uname),Darwin) # MacOS 
		CROSS_COMPILE	:= i386-elf-
	endif
	
	ENV_LD		:=  $(CROSS_COMPILE)ld -m elf_i386
	ENV_USER_LD_SCRIPT	:= -T ../libs/xlibc/arch/x86/user.ld
	ENV_AS		:= nasm
	ENV_LDFLAGS	:= -no-pie
	ENV_AFLAGS	+= -f elf 
	CFLAGS		:= -march=i386 -m32 
	CFLAGS		+= -O3
	CFLAGS		+= -std=gnu99
	CFLAGS		+= -fno-PIE
else ifeq ($(ENV_ARCH), riscv64)
	ENV_LD		:=  $(CROSS_COMPILE)ld 
	ENV_USER_LD_SCRIPT	:= -T ../libs/xlibc/arch/riscv64/user.ld
	ENV_AS		:= $(CROSS_COMPILE)gcc -x assembler-with-cpp
	MCFLAGS		:= -march=rv64imafdc -mabi=lp64d -mcmodel=medany
	ENV_AFLAGS	+= -ffunction-sections -fdata-sections -ffreestanding -std=gnu99 
	CFLAGS		+= -fno-omit-frame-pointer
	CFLAGS		+= -O
	CFLAGS 		+= -MD
	CFLAGS 		+= -ffreestanding -fno-common
	CFLAGS 		+= -mno-relax
	CFLAGS		+= -std=gnu99
	ifeq ($(ENV_MACH), mach-qemu)
	CFLAGS 		+= -D QEMU
	endif
endif 

ENV_AFLAGS	+= $(MCFLAGS)

ENV_CFLAGS	:=$(MCFLAGS)
ENV_CFLAGS	+= $(CFLAGS)
ENV_CFLAGS	+= -Wall -Wunused 
ENV_CFLAGS	+= -fno-builtin -fno-stack-protector -fno-strict-aliasing -nostdlib

# kernel name & version
ENV_CFLAGS	+= -DKERNEL_NAME=\"xbook2\" -DKERNEL_VERSION=\"0.1.9\"