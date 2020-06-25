# makefile for bosh

#The tools name
NASM 		= nasm
CC			= gcc
LD 			= ld
DD 			= dd
OBJDUMP		= objdump
FATFS 		= ../../tools/fatfs/fatfs

#The file path
DIR_LIBARY = ../../libary

#rom dir
DIR_ROM = ../../develop/rom

# libarys
DIR_XLIBC = $(DIR_LIBARY)/xlibc
DIR_SGI = $(DIR_LIBARY)/sgi

BIN = $(DIR_ROM)/bin/bosh
BIN_DIS = out.disasm

# rom disk size (MB)
ROM_DISK_SZ = 10	

#img dir
IMG_DIR = ../../develop/image/

DISK_IMG = d.img
DISK_VHD = d.vhd
DISK_VMDK = d-flat.vmdk

HD_IMG = $(IMG_DIR)$(DISK_IMG)
HD_VHD = $(IMG_DIR)$(DISK_VHD)
HD_VMDK = $(IMG_DIR)$(DISK_VMDK)

#flags
ASM_FLAGS	= -f elf \
			-I $(DIR_XLIBC)/indlude 

C_FLAGS		= -c -fno-builtin -Wall -Wunused -m32 \
			-I $(DIR_XLIBC)/include \
			-I $(DIR_SGI)/include 

LDFLAGS		= -m elf_i386 -e _start -Ttext 0x00001000

OBJS =  main.o \
		window.o \
		console.o \
		cmd.o \
		cursor.o \
		clipboard.o \
		
LIB_FILE := $(DIR_SGI)/sgi-0.0.1.a \
			$(DIR_XLIBC)/xlibc-0.0.1.a \
			
#First read here
.PHONY: all dis

all: compile link rom

#Compile files
compile: $(OBJS)

link: $(BIN)
$(BIN): $(OBJS) $(LIB_FILE)
	$(LD) $(LDFLAGS) -o $(BIN) $(OBJS) $(LIB_FILE)

dis: $(BIN)
	$(OBJDUMP) -M intel -D $(BIN) > $(BIN_DIS)

# write rom files
rom:
	$(FATFS) $(HD_IMG) $(DIR_ROM) $(ROM_DISK_SZ)
	$(FATFS) $(HD_VHD) $(DIR_ROM) $(ROM_DISK_SZ)
	$(FATFS) $(HD_VMDK) $(DIR_ROM) $(ROM_DISK_SZ)

#Clean temporary files
clean:
	-rm *.o
	-rm $(BIN)

%.o: %.asm
	$(NASM) $(ASM_FLAGS) -o $@ $<
	
%.o: %.c
	$(CC) $(C_FLAGS) -o $@ $<
