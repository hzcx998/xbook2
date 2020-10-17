sinclude ../scripts/env.mk

X_ASFLAGS	+= -f elf -I $(DIR_XLIBC)/indlude 
X_CFLAGS    += -fno-builtin -Wall -Wunused -fno-PIE -m32 -fno-stack-protector -std=c99 -O2
X_INCDIRS	+= ../library/xlibc/include ../library/pthread/include ../library/jpeg \
				../library/libpng ../library/zlib ../library/gapi/include \
				../library/xlibcpp/include ../library/microui ../library/cjson ../library/lvgl \
				../library/gato

X_LDFLAGS	+= -no-pie -m elf_i386 -e _start -Ttext 0x00001000
X_LIBDIRS	+= $(srctree)/../library/xlibc $(srctree)/../library/pthread \
				$(srctree)/../library/jpeg $(srctree)/../library/libpng \
				$(srctree)/../library/zlib $(srctree)/../library/gapi \
				$(srctree)/../library/xlibcpp $(srctree)/../library/microui \
				$(srctree)/../library/cjson $(srctree)/../library/lvgl \
				$(srctree)/../library/gato

LD			:= ld -m elf_i386
AS			:=	nasm

export AS LD

# bin
MODULE      +=  echo
MODULE      +=  cat
MODULE      +=  cpuinfo
MODULE      +=  ls
MODULE      +=  cp
MODULE      +=  rm
MODULE      +=  ps
MODULE      +=  mem
MODULE      +=  date
MODULE      +=  mv
MODULE      +=  rename
MODULE      +=  mkdir
MODULE      +=  rmdir
MODULE      +=  trig
MODULE      +=  touch
MODULE      +=  sys

# usr
MODULE      +=  freeterm
MODULE      +=  lvgl
MODULE      +=  lvgl2
MODULE      +=  microui
MODULE      +=  cpptests
MODULE      +=  lua
MODULE      +=  cal
MODULE      +=  invader
MODULE      +=  gato
MODULE      +=  bosh
MODULE      +=  vt100
MODULE      +=  tests
MODULE      +=  hello
MODULE      +=  socket_client
MODULE      +=  socket_server
MODULE      +=  win
MODULE      +=  infones
MODULE      +=  c4
MODULE      +=  sound
MODULE      +=  wavplayer
MODULE      +=  mp3player