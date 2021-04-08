sinclude ../scripts/env.mk

X_ASFLAGS	+= -f elf -I $(DIR_XLIBC)/indlude 
X_CFLAGS	:= -fno-builtin -Wall -Wunused -fno-PIE -m32 -fno-stack-protector -std=gnu99 -O3

ifeq ($(shell uname),Darwin)
	LD			:=  i386-elf-ld -m elf_i386
else
	LD			:=  ld -m elf_i386
endif

AS			:= nasm
export AS LD

MODULE		:= xlibc
MODULE		+= xlibcpp
MODULE		+= sconf
MODULE		+= pthread
MODULE		+= netsocket
MODULE		+= uview