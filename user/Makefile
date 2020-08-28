sinclude ../scripts/env.mk

X_ASFLAGS	+= -f elf -I $(DIR_XLIBC)/indlude 
X_CFLAGS    += -fno-builtin -Wall -Wunused -fno-PIE -m32 -fno-stack-protector -std=c99
X_INCDIRS	+= ../library/xlibc/include ../library/pthread/include ../library/jpeg \
				../library/libpng ../library/zlib ../library/gapi/include \ 

X_LDFLAGS	+= -no-pie -m elf_i386 -e _start -Ttext 0x00001000
X_LIBDIRS	+= $(srctree)/../library/xlibc $(srctree)/../library/pthread \
				$(srctree)/../library/jpeg $(srctree)/../library/libpng \
				$(srctree)/../library/zlib $(srctree)/../library/gapi

AS			:=	nasm

export AS

MODULE      +=  vt100
MODULE      +=  tests
MODULE      +=  hello
MODULE      +=  bosh
MODULE      +=  socket_client
MODULE      +=  socket_server
MODULE      +=  echo
MODULE      +=  cat
MODULE      +=  win
MODULE      +=  infones
