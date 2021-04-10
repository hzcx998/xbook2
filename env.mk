# env var 
ENV_CFLAGS	:= -march=i386 -fno-builtin -Wall -Wunused -fno-PIE -m32 -std=gnu99 -O3 -fno-stack-protector 
ENV_AFLAGS	:= -f elf 
ENV_LDFLAGS	:= -m elf_i386 -no-pie 

ENV_USER_LD_SCRIPT	:= ../libs/xlibc/arch/x86/user.ld

# MacOS specail
ifeq ($(shell uname),Darwin)
	ENV_LD		:=  i386-elf-ld
else
	ENV_LD		:=  ld
endif

ENV_AS		:= nasm