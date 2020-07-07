sinclude ../scripts/env.mk

X_ASFLAGS	+= -f elf -I $(DIR_XLIBC)/indlude 
X_CFLAGS    += -fno-builtin -Wall -Wunused -fno-PIE -m32 -fno-stack-protector
X_INCDIRS	+= ../library/xlibc/include ../library/sgi/include ../library/pthread/include 
X_LDFLAGS	+= -no-pie -m elf_i386 -e _start -Ttext 0x00001000
X_LIBDIRS	+= $(srctree)/../library/xlibc $(srctree)/../library/sgi $(srctree)/../library/pthread 

AS			:=	nasm

export AS

MODULE      +=  bosh
MODULE      +=  infones
MODULE      +=  test