# makefile for test

#The tools name
NASM 		= nasm
CC			= gcc
LD 			= ld
DD 			= dd
OBJDUMP		= objdump

#The file path
DIR_LIBARY = ../../library

# librarys
DIR_XLIBC 		= $(DIR_LIBARY)/xlibc
DIR_PTHREAD 	= $(DIR_LIBARY)/pthread

BIN = a.out
BIN_DIS = out.disasm
BIN_OFF = 400 
BIN_CNTS = 400

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
			-I $(DIR_XLIBC)/include \
			-I $(DIR_PTHREAD)/include \
			-I include \
			-I fatfs

C_FLAGS		= -c -fno-builtin -Wall -Wunused -fno-stack-protector -m32 \
			-I $(DIR_XLIBC)/include \
			-I $(DIR_PTHREAD)/include \
			-I include \
			-I fatfs
			
FATFS_OBJS = 	fatfs/diskio.o \
				fatfs/ff.o \
				fatfs/ffsystem.o \
				fatfs/ffunicode.o \

FSAL_OBJS = 	fsal/fsal.o \
				fsal/fatfs.o \
				fsal/path.o \
				fsal/interface.o \
				fsal/dir.o \

DRIVERS_OBJS 	=  drivers/disk.o \

CORE_OBJS 	=   core/interface.o \
				core/init.o \
				core/fstype.o \

MAIN_OBJS =  main.o \

OBJS =  $(MAIN_OBJS) $(CORE_OBJS) $(DRIVERS_OBJS) $(FATFS_OBJS) $(FSAL_OBJS)


LIB_FILE = 	$(DIR_XLIBC)/xlibc-0.0.1.a \

# -n: close memory aligin.
LDFLAGS		= -no-pie -m elf_i386 -e _start -Ttext 0x00001000

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

#disk: img vhd vmdk floppy
disk: img

img:
	$(DD) if=$(BIN) of=$(HD_IMG) bs=512 seek=$(BIN_OFF) count=$(BIN_CNTS) conv=notrunc

vhd:
	$(DD) if=$(BIN) of=$(HD_VHD) bs=512 seek=$(BIN_OFF) count=$(BIN_CNTS) conv=notrunc
	
vmdk:
	$(DD) if=$(BIN) of=$(HD_VMDK) bs=512 seek=$(BIN_OFF) count=$(BIN_CNTS) conv=notrunc

floppy:
	$(DD) if=$(BIN) of=$(FD_IMG) bs=512 seek=$(FLOPPY_OFF) count=$(BIN_CNTS) conv=notrunc
	
phy: compile link floppy
	
#Clean temporary files
clean:
	-rm $(OBJS)
	-rm $(BIN)

%.o: %.asm
	$(NASM) $(ASM_FLAGS) -o $@ $<
	
%.o: %.c
	$(CC) $(C_FLAGS) -o $@ $<
