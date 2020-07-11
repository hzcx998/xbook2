sinclude ../scripts/env.mk

X_ASFLAGS	+= -f elf -I $(DIR_XLIBC)/indlude 
X_CFLAGS    += -fno-builtin -Wall -Wunused -fno-PIE -m32 -fno-stack-protector -std=c99
X_INCDIRS	+= ../library/xlibc/include ../library/sgi/include ../library/pthread/include ../library/jpeg 
X_LDFLAGS	+= -no-pie -m elf_i386 -e _start -Ttext 0x00001000
X_LIBDIRS	+= $(srctree)/../library/xlibc $(srctree)/../library/sgi $(srctree)/../library/pthread \
				$(srctree)/../library/jpeg 

AS			:=	nasm

export AS

MODULE      +=  tests
