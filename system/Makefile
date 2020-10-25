sinclude ../scripts/env.mk

X_ASFLAGS	+= -f elf -I $(DIR_XLIBC)/indlude 
X_CFLAGS    += -fno-builtin -Wall -Wunused -fno-PIE -m32 -fno-stack-protector -std=gnu99
X_INCDIRS	+= ../library/xlibc/include ../library/jpeg ../library/libpng ../library/gapi/include \
				../library/zlib ../library/iniparser

X_LDFLAGS	:= -no-pie -m elf_i386 -e _start -Ttext 0x00001000
X_LIBDIRS	+= ../library/xlibc ../library/jpeg ../library/libpng ../library/gapi ../library/zlib \
				../library/iniparser

LD			:= ld -m elf_i386
AS			:=	nasm

export AS LD

MODULE      +=  init
MODULE      +=  sh
