sinclude ../scripts/env.mk

X_ASFLAGS	+= -f elf -I $(DIR_XLIBC)/indlude 
X_CFLAGS    += -fno-builtin -Wall -Wunused -fno-PIE -m32 -fno-stack-protector -std=gnu99
X_INCDIRS	+= ../library/xlibc/include ../library/jpeg

X_LDFLAGS	:= -no-pie -m elf_i386 -e _start -Ttext 0x00001000
X_LIBDIRS	+= ../library/xlibc ../library/jpeg

AS			:=	nasm

export AS

MODULE      +=  init
MODULE      +=  desktop
