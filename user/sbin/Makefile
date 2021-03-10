sinclude ../../scripts/env.mk

LIBS_DIR	:= ../libs

X_ASFLAGS	+= -f elf -I $(DIR_XLIBC)/indlude 
X_CFLAGS    += -fno-builtin -Wall -Wunused -fno-PIE -m32 -fno-stack-protector -std=gnu99
X_INCDIRS	+= $(LIBS_DIR)/xlibc/include \
				$(LIBS_DIR)/pthread/include \
				$(LIBS_DIR)/netsocket/include \
				$(LIBS_DIR)/uview/include \
	
X_LDFLAGS	:= -no-pie -m elf_i386 -e _start -Ttext 0x00001000
X_LIBDIRS	+= $(LIBS_DIR)/xlibc \
				$(LIBS_DIR)/netsocket \
				$(LIBS_DIR)/pthread \
				$(LIBS_DIR)/uview \

LD			:= ld -m elf_i386
AS			:=	nasm

export AS LD

MODULE      +=  init
MODULE      +=  login
MODULE      +=  netserv
