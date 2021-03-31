sinclude ../../scripts/env.mk

X_ASFLAGS	+= -f elf -I $(DIR_XLIBC)/indlude 
X_CFLAGS	:= -fno-builtin -Wall -Wunused -fno-PIE -m32 -fno-stack-protector -std=gnu99

LD			:= ld -m elf_i386
AS			:= nasm
export AS LD

MODULE		:= xlibc
MODULE		+= xlibcpp
MODULE		+= pthread
MODULE		+= netsocket
MODULE		+= jpeg
MODULE		+= libpng
MODULE		+= zlib
MODULE		+= lvgl
MODULE		+= uview

