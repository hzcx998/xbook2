# env var

ENV_CFLAGS	:= -march=i386 -fno-builtin -Wall -Wunused -fno-PIE -m32 -std=gnu99 -O3 -fno-stack-protector
# kernel name & version
ENV_CFLAGS	+= -DKERNEL_NAME=\"xbook2\" -DKERNEL_VERSION=\"0.1.9\"

ENV_AFLAGS	:= -f elf
ENV_LDFLAGS	:= -no-pie

ENV_USER_LD_SCRIPT	:= -T ../libs/xlibc/arch/x86/user.ld

# MacOS special
ifeq ($(shell uname),Darwin)
	ENV_LD		:=  i386-elf-ld -m elf_i386
else
	ENV_LD		:=  ld -m elf_i386
endif

ENV_AS		:= nasm