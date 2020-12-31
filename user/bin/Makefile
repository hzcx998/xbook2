sinclude ../../scripts/env.mk

LIBS_DIR	:= ../libs

X_ASFLAGS	+= -f elf -I $(DIR_XLIBC)/indlude 
X_CFLAGS    += -fno-builtin -Wall -Wunused -fno-PIE -m32 -fno-stack-protector -std=gnu99 -O3
X_INCDIRS	+= $(LIBS_DIR)/xlibc/include $(LIBS_DIR)/pthread/include $(LIBS_DIR)/netsocket/include \
				$(LIBS_DIR)/xgui/include \
				$(LIBS_DIR)/zlib \
				$(LIBS_DIR)/libpng \
				$(LIBS_DIR)/jpeg \
				$(LIBS_DIR)/lvgl \
				$(LIBS_DIR)/lvgl/include \
				
X_LDFLAGS	+= -no-pie -m elf_i386 -e _start -Ttext 0x00001000
X_LIBDIRS	+= $(LIBS_DIR)/xlibc $(LIBS_DIR)/pthread \
				$(LIBS_DIR)/netsocket \
				$(LIBS_DIR)/xgui \
				$(LIBS_DIR)/zlib \
				$(LIBS_DIR)/libpng \
				$(LIBS_DIR)/jpeg \
				$(LIBS_DIR)/lvgl \
				
LD			:= ld -m elf_i386
AS			:=	nasm

export AS LD

# bin
MODULE      +=  echo
MODULE      +=  cat
MODULE      +=  cal
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
MODULE      +=  touch
MODULE      +=  tests
MODULE      +=  hello
MODULE      +=  osver
MODULE      +=  lua
MODULE      +=  sh
MODULE      +=  xbrower