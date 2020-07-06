sinclude ../scripts/env.mk

X_ASFLAGS	+= -f elf -I $(DIR_XLIBC)/indlude 
X_CFLAGS    += -fno-builtin -Wall -Wunused -fno-PIE -m32 -fno-stack-protector
X_INCDIRS	+= ../libary/xlibc/include ../libary/sgi/include ../libary/pthread/include 
X_LDFLAGS	+= -no-pie -m elf_i386 -e _start -Ttext 0x00001000
X_LIBDIRS	+= $(srctree)/../libary/xlibc $(srctree)/../libary/sgi $(srctree)/../libary/pthread 

AS			:=	nasm

export AS

MODULE      +=  bosh
MODULE      +=  infones
MODULE      +=  test