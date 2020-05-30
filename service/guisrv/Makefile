# makefile for test

#The tools name
NASM 		= nasm
CC			= gcc
LD 			= ld
DD 			= dd
OBJDUMP		= objdump

#The file path
DIR_LIBARY = ../../libary

DIR_INCLUDE = ./include

# libarys
DIR_XBOOKLIB 		= $(DIR_LIBARY)/xbooklib

# rom dir
ROM_DIR = ../../develop/rom

BIN = a.out
BIN_DIS = a.out.disasm
BIN_OFF = 800 
BIN_CNTS = 700

FLOPPY_OFF = 522

#img dir
IMG_DIR = ../../develop/image/

FLOPPY_IMG = a.img
DISK_IMG = c.img
DISK_VHD = c.vhd
DISK_VMDK = c-flat.vmdk

FD_IMG = $(IMG_DIR)$(FLOPPY_IMG)
HD_IMG = $(IMG_DIR)$(DISK_IMG)
HD_VHD = $(IMG_DIR)$(DISK_VHD)
HD_VMDK = $(IMG_DIR)$(DISK_VMDK)


#flags
ASM_FLAGS	= -f elf \
			-I $(DIR_INCLUDE) \
			-I $(DIR_XBOOKLIB)/include \

C_FLAGS		= -c -fno-builtin -Wall -Wunused -m32 \
			-I $(DIR_INCLUDE) \
			-I $(DIR_XBOOKLIB)/include \

DRIVERS_OBJS 	= 	drivers/screen.o \
					drivers/mouse.o \
					drivers/keyboard.o \

LAYER_OBJS 	= 		layer/point.o \
					layer/line.o \
					layer/rect.o \
					layer/bitmap.o \
					layer/layer.o \

WINDOW_OBJS 	=	window/window.o \
					window/point.o \

MAIN_OBJS 		= 	main.o \
				
OBJS =  $(MAIN_OBJS) $(DRIVERS_OBJS) $(LAYER_OBJS) $(WINDOW_OBJS)


LIB_FILE 	= $(DIR_XBOOKLIB)/xbooklib-0.0.1.a \

LDFLAGS		= -m elf_i386 -e _start -Ttext 0x00001000

#First read here
.PHONY: all dis

all: compile link disk 

#Compile files
compile: $(OBJS)

link: $(BIN)
$(BIN): $(OBJS) $(LIB_FILE)
	$(LD) $(LDFLAGS) -o $(BIN) $(OBJS) $(LIB_FILE) 


dis: $(BIN)
	$(OBJDUMP) -M intel -D $(BIN) > $(BIN_DIS)

disk: img vhd vmdk floppy

img:
	$(DD) if=$(BIN) of=$(HD_IMG) bs=512 seek=$(BIN_OFF) count=$(BIN_CNTS) conv=notrunc
	
vhd:
	$(DD) if=$(BIN) of=$(HD_VHD) bs=512 seek=$(BIN_OFF) count=$(BIN_CNTS) conv=notrunc
	
vmdk:
	$(DD) if=$(BIN) of=$(HD_VMDK) bs=512 seek=$(BIN_OFF) count=$(BIN_CNTS) conv=notrunc

floppy:
	$(DD) if=$(BIN) of=$(FD_IMG) bs=512 seek=$(FLOPPY_OFF) count=$(BIN_CNTS) conv=notrunc
	
qemu: compile link img
	
vbox: compile link vhd

vmware: compile link vmdk

phy: compile link floppy
	
#Clean temporary files
clean:
	-rm *.o
	-rm $(BIN)

%.o: %.asm
	$(NASM) $(ASM_FLAGS) -o $@ $<
	
%.o: %.c
	$(CC) $(C_FLAGS) -o $@ $<
