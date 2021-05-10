# env var 

CROSS_COMPILE 	?= 
#CROSS_COMPILE ?= riscv-none-embed-
PLATFORM		?=

#
# Get platform information about ARCH and MACH from PLATFORM variable.
#
ifeq ($(words $(subst -, , $(PLATFORM))), 2)
ENV_ARCH			:= $(word 1, $(subst -, , $(PLATFORM)))
ENV_MACH			:= mach-$(word 2, $(subst -, , $(PLATFORM)))
else
ENV_ARCH			:= x86
ENV_MACH			:= mach-i386
endif

# MacOS must need i386-elf-	
ifeq ($(ENV_ARCH), x86)
	ifeq ($(shell uname),Darwin) # MacOS 
		CROSS_COMPILE	:= i386-elf-
	endif
	ENV_CFLAGS_MACH	:= -march=i386 -m32 
	ENV_LD		:=  $(CROSS_COMPILE)ld -m elf_i386
	ENV_USER_LD_SCRIPT	:= -T ../libs/xlibc/arch/x86/user.ld
	ENV_AS		:= nasm
endif 

ENV_CFLAGS	:= $(ENV_CFLAGS_MACH) 
ENV_CFLAGS	+= -fno-builtin -Wall -Wunused 
ENV_CFLAGS	+= -fno-PIE -fno-stack-protector  -fno-strict-aliasing -nostdlib
ENV_CFLAGS	+= -std=gnu99
ENV_CFLAGS	+= -O3

# kernel name & version
ENV_CFLAGS	+= -DKERNEL_NAME=\"xbook2\" -DKERNEL_VERSION=\"0.1.9\"

ENV_AFLAGS	:= -f elf 
ENV_LDFLAGS	:= -no-pie