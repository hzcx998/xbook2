# env var 
# $(warning $(xxx))

# 是否为远程测试，yes or no,根据要求，如果是远程测试的话，就需要通过make all生成内核，以及
# 在当前目录下面生成k210.bin可执行文件即可。因此就不需要构建用户态的内容以及写入磁盘和运行内核的工作。
ENV_REMOTE_TEST	:=yes

# 默认的CROSS_COMPILE和PLATFORM
#CROSS_COMPILE 	?= 
#PLATFORM		?=
#CROSS_COMPILE 	?= riscv-none-embed-
#CROSS_COMPILE 	?= riscv64-linux-gnu-
CROSS_COMPILE 	?= riscv64-unknown-elf-
PLATFORM		?= riscv64-qemu

# 如果是远程测试，就需要强制使用对应的编译环境以及开发平台 
ifeq ($(ENV_REMOTE_TEST),yes)
CROSS_COMPILE	:= riscv64-unknown-elf-
PLATFORM		:= riscv64-k210
endif

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
	ENV_LIBC	:= xlibc
	ENV_LD		:=  $(CROSS_COMPILE)ld -m elf_i386
	ENV_USER_LD_SCRIPT	:= -T ../libs/xlibc/arch/x86/user.ld
	ENV_AS		:= nasm
	ENV_LDFLAGS	:= -no-pie
	ENV_AFLAGS	:= -f elf 
	CFLAGS		:= -march=i386 -m32 
	CFLAGS		+= -O3
	CFLAGS		+= -std=gnu99
	CFLAGS		+= -fno-PIE
	CFLAGS		+= -DCONFIG_32BIT
	CFLAGS		+= -D__X86__
	CFLAGS		+= -D__XLIBC__
else ifeq ($(ENV_ARCH), riscv64)
#ENV_LIBC	:= xlibc
	ENV_LIBC	:= tinylibc

	ENV_LD		:=  $(CROSS_COMPILE)ld 
	ENV_AS		:= $(CROSS_COMPILE)gcc -x assembler-with-cpp
	MCFLAGS		:= -march=rv64imafdc -mabi=lp64d -mcmodel=medany
	ENV_AFLAGS	:= -ffunction-sections -fdata-sections -ffreestanding -std=gnu99 
	CFLAGS		+= -fno-omit-frame-pointer
	CFLAGS		+= -O0 -g -ggdb
	CFLAGS 		+= -MD
	CFLAGS 		+= -ffreestanding -fno-common
	CFLAGS 		+= -mno-relax
	CFLAGS		+= -std=gnu99
	ifeq ($(ENV_MACH), mach-qemu)
	CFLAGS 		+= -D QEMU
	endif
	CFLAGS		+= -DCONFIG_64BIT
	CFLAGS		+= -D__RISCV64__
ifeq ($(ENV_LIBC),xlibc)
	ENV_USER_LD_SCRIPT	:= -T ../libs/xlibc/arch/riscv64/user.ld
	CFLAGS		+= -D__XLIBC__
else ifeq ($(ENV_LIBC),tinylibc)
	ENV_USER_LD_SCRIPT	:= -T ../libs/tinylibc/arch/riscv/user.ld
	CFLAGS		+= -D__TINYLIBC__
endif

endif 

ENV_AFLAGS	+= $(MCFLAGS)

ENV_CFLAGS	:=$(MCFLAGS)
ENV_CFLAGS	+= $(CFLAGS)
ENV_CFLAGS	+= -Wall -Wunused 
ENV_CFLAGS	+= -fno-builtin -fno-stack-protector -fno-strict-aliasing -nostdlib -nostdinc

# kernel name & version
ENV_CFLAGS	+= -DKERNEL_NAME=\"xbook2\" -DKERNEL_VERSION=\"0.1.9\"