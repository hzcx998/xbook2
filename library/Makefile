sinclude ../scripts/env.mk

X_ASFLAGS	+= -f elf -I $(DIR_XLIBC)/indlude 
X_CFLAGS	:= -fno-builtin -Wall -Wunused -fno-PIE -m32 -fno-stack-protector -std=gnu99

LD			:= ld -m elf_i386
AS			:= nasm
export AS LD

MODULE		:= xlibc
MODULE		+= gato
MODULE		+= lvgl
MODULE		+= cjson
MODULE		+= microui
MODULE		+= xlibcpp
MODULE		+= pthread
MODULE		+= zlib
MODULE		+= libpng
MODULE		+= jpeg
MODULE		+= gapi
MODULE		+= iniparser
