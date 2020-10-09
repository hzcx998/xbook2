sinclude ../scripts/env.mk

X_ASFLAGS	+= -f elf -I $(DIR_XLIBC)/indlude 
X_CFLAGS	:= -fno-builtin -Wall -Wunused -fno-PIE -m32 -fno-stack-protector -std=gnu99

AS			:= nasm
export AS

MODULE		:= xlibc
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
