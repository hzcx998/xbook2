# makefile for xui

#The tools name
NASM 		= nasm
CC			= gcc
LD 			= ld
DD 			= dd
OBJDUMP		= objdump
COPY		= cp

#The file path
DIR_LIBARY = ../../libary

DIR_INCLUDE = ./include

# libarys
DIR_XLIBC 		= $(DIR_LIBARY)/xlibc
DIR_FREETYPE 	= $(DIR_LIBARY)/freetype-2.10.1
DIR_JPEG 		= $(DIR_LIBARY)/jpeg-9d
DIR_LIBPNG 		= $(DIR_LIBARY)/libpng-1.6.37
DIR_ZLIB 		= $(DIR_LIBARY)/zlib-1.2.11
DIR_SGI 		= $(DIR_LIBARY)/sgi

# rom dir
DIR_ROM = ../../develop/rom

BIN = $(DIR_ROM)/sbin/xui
BIN_DIS = a.disasm

#flags
ASM_FLAGS	= -f elf \
			-I $(DIR_INCLUDE) \
			-I $(DIR_XLIBC)/include \

C_FLAGS		= -c -fno-builtin -Wall -Wunused -fno-stack-protector -m32 -std=gnu99 \
			-I $(DIR_INCLUDE) \
			-I $(DIR_XLIBC)/include \
			-I $(DIR_FREETYPE)/include \
			-I $(DIR_JPEG) \
			-I $(DIR_LIBPNG) \
			-I $(DIR_ZLIB) \
			-I $(DIR_SGI)/include \

DRIVERS_OBJS 	= 	drivers/screen.o \
					drivers/mouse.o \
					drivers/keyboard.o \
					drivers/fb-sandbox.o \
					drivers/sandbox-fb.o \
					
GRAPHIC_OBJS 	= 	graphic/color.o \
					graphic/font.o \
					graphic/icon.o \
					graphic/matrix.o \
					graphic/region.o \
					graphic/render.o \
					graphic/surface.o \
					graphic/svg.o \
					graphic/svg-raster.o \
					graphic/text.o \
					
XFS_OBJS 		= 	xfs/archiver.o \
					xfs/archiver-dir.o \
					xfs/archiver-tar.o \
					xfs/xfs.o \
				
XUI_OBJS 		= 	xui/xui.o \
					xui/xui-badge.o \
					xui/xui-button.o \
					xui/xui-checkbox.o \
					xui/xui-colorpicker.o \
					xui/xui-label.o \
					xui/xui-number.o \
					xui/xui-panel.o \
					xui/xui-popup.o \
					xui/xui-progress.o \
					xui/xui-radialbar.o \
					xui/xui-radio.o \
					xui/xui-slider.o \
					xui/xui-spinner.o \
					xui/xui-split.o \
					xui/xui-text.o \
					xui/xui-textedit.o \
					xui/xui-toggle.o \
					xui/xui-tree.o \
					xui/xui-window.o \

FRAMEBUFFER_OBJS= 	framebuffer/framebuffer.o \

CORE_OBJS		= 	core/window.o \

MAIN_OBJS 		= 	main.o \

OBJS =  $(MAIN_OBJS) $(DRIVERS_OBJS) $(GRAPHIC_OBJS) $(XFS_OBJS) $(XUI_OBJS) $(FRAMEBUFFER_OBJS) \
		$(CORE_OBJS)

LIB_FILE 	:=  $(DIR_FREETYPE)/freetype-2.10.1.a \
				$(DIR_JPEG)/jpeg-9d.a \
				$(DIR_LIBPNG)/libpng-1.6.37.a \
				$(DIR_ZLIB)/zlib-1.2.11.a \
				$(DIR_SGI)/sgi-0.0.1.a \
				$(DIR_XLIBC)/xlibc-0.0.1.a \
			
# -n: close memory aligin.
#LDFLAGS		= -T link.ld -n
LDFLAGS		= -no-pie -m elf_i386 -e _start -Ttext 0x00001000

#First read here
.PHONY: all dis

all: compile link

#Compile files
compile: $(OBJS)

link: $(BIN) 
$(BIN): $(OBJS) $(LIB_FILE)
	$(LD) $(LDFLAGS) $(OBJS) $(LIB_FILE) -o $(BIN) 


dis: $(BIN)
	$(OBJDUMP) -M intel -D $(BIN) > $(BIN_DIS)

#Clean temporary files
clean:
	-rm $(OBJS)
	-rm $(BIN)

%.o: %.asm
	$(NASM) $(ASM_FLAGS) -o $@ $<
	
%.o: %.c
	$(CC) $(C_FLAGS) -o $@ $<
