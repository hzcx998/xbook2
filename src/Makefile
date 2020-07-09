# MIT License
# Copyright (c) 2020 Jason Hu, Zhu Yu
# 编译内核

#----tools----
#tool dir

PHONY		:=
all:

# environment
NASM 		= nasm
CC			= gcc
LD 			= ld
OBJDUMP		= objdump
OBJCOPY		= objcopy

# arch
ARCH_DIR	= arch/x86

#----flags----
#out a map file: -Map kernel.map
#i386 link flags
LDFLAGS		= -no-pie -m elf_i386

TOPDIR	:= $(CURDIR)

# System environment variable.
ifeq ($(OS),Windows_NT)
	HOSTOS		:= windows
else
	ifeq ($(shell uname),Darwin)
		HOSTOS		:= macos
	else
		ifeq ($(shell uname),Linux)
			HOSTOS		:= linux
		else
			HOSTOS		:= unix-like
		endif
	endif
endif

#objs
SRCDIRS	:= $(ARCH_DIR)/ init/ ipc/ task/ vmm/ driver/ debug/ lib/ kernel/

INCDIRS	:= include/

OBJS	:= $(patsubst %/, %/built-in.o, $(SRCDIRS))

INCLUDE_PATH	:= $(patsubst %, -I $(TOPDIR)/%, $(INCDIRS))

OPTFLAGS		:=
ifeq ("$(origin G)", "command line")
ifeq ($(G),on)
OPTFLAGS		:= -O0 -g -ggdb
endif
endif

CFLAGS	:=  -fno-builtin -nostdinc -Wall -Wunused -fno-PIE -m32 -fno-stack-protector $(OPTFLAGS) $(INCLUDE_PATH) -c
NASM_FLAGS	:= $(INCLUDE_PATH)

OBJCOPY_FLAGS = -R .note -R .comment -S 

#kernel file	
KERNEL_ELF =  kernel.elf
KERNEL_ELF_DISASM =  kernel.elf.disasm

PHONY	+= all run compile link clean dis
PHONY	+= $(SRCDIRS)

export NASM NASM_FLAGS CC LD CFLAGS LDFLAGS TOPDIR OBJCOPY OBJCOPY_FLAGS

#----cmd----
all: $(KERNEL_ELF)

#compile file
$(OBJS) : $(SRCDIRS) ;

$(SRCDIRS) :
ifneq ($(strip $(MAKECMDGOALS)),clean)
	$(MAKE) -C $@ -f $(TOPDIR)/Makefile.build
else
	$(MAKE) -C $@ -f $(TOPDIR)/Makefile.build clean
endif

# -n: close memory aligin.
# -s: strip-all symbol
$(KERNEL_ELF): $(OBJS)
	@echo [LD] with $(ARCH_DIR)/x86-kernel.ld output $@
	$(LD) -n -T $(ARCH_DIR)/x86-kernel.ld -o $(KERNEL_ELF) $(OBJS)
	
#$(LD) $(LDFLAGS)  -e _start -Ttext 0x80100000 -o $(KERNEL_ELF) $(OBJS)

dis: $(KERNEL_ELF)
	$(OBJDUMP) -M intel -D $(KERNEL_ELF) > $(KERNEL_ELF_DISASM)

# clean temporary files
clean: $(SRCDIRS)
	$(RM) $(KERNEL_ELF) $(KERNEL_ELF_DISASM)

.PHONY: $(PHONY)
