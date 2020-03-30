# makefile for test

#The tools name
NASM 		= nasm
CC			= gcc
LD 			= ld
DD 			= dd
OBJDUMP		= objdump

#The file path
DIR_LIBARY = ../../libary

# base lib
DIR_BASELIB = $(DIR_LIBARY)/base
DIR_BASELIB_INCLUDE = $(DIR_BASELIB)/include
# xlibc lib
DIR_XLIBC = $(DIR_LIBARY)/xlibc
DIR_XLIBC_INCLUDE = $(DIR_XLIBC)/include

BIN = a.out
BIN_DIS = out.disasm
BIN_OFF = 200 
BIN_CNTS = 100

#img dir
IMG_DIR = ../../develop/image/

DISK_IMG = c.img
DISK_VHD = c.vhd
DISK_VMDK = c-flat.vmdk

HD_IMG = $(IMG_DIR)$(DISK_IMG)
HD_VHD = $(IMG_DIR)$(DISK_VHD)
HD_VMDK = $(IMG_DIR)$(DISK_VMDK)

#flags
#flags
ASM_FLAGS	= -f elf \
			-I $(DIR_BASELIB_INCLUDE) \
			-I $(DIR_XLIBC_INCLUDE)
			
C_FLAGS		= -c -fno-builtin -Wall -Wunused -m32 \
			-I $(DIR_BASELIB_INCLUDE) \
			-I $(DIR_XLIBC_INCLUDE)


LDFLAGS		= -m elf_i386 -e _start -Ttext 0x00001000

OBJS =  _start.o \
		main.o \

LIB_OBJS = 	$(DIR_BASELIB)/baselib.a  \
			$(DIR_XLIBC)/xlibc.a

#First read here
.PHONY: all dis

all: compile link disk 

#Compile files
compile: $(OBJS)

link: $(BIN)
$(BIN): $(OBJS) $(LIB_OBJS)
	$(LD) $(LDFLAGS) -o $(BIN) $(OBJS) $(LIB_OBJS)


dis: $(BIN)
	$(OBJDUMP) -M intel -D $(BIN) > $(BIN_DIS)

disk: img vhd vmdk

img:
	$(DD) if=$(BIN) of=$(HD_IMG) bs=512 seek=$(BIN_OFF) count=$(BIN_CNTS) conv=notrunc
	
vhd:
	$(DD) if=$(BIN) of=$(HD_VHD) bs=512 seek=$(BIN_OFF) count=$(BIN_CNTS) conv=notrunc
	
vmdk:
	$(DD) if=$(BIN) of=$(HD_VMDK) bs=512 seek=$(BIN_OFF) count=$(BIN_CNTS) conv=notrunc
	
qemu: compile link img
	
vbox: compile link vhd

vmware: compile link vmdk
	
#Clean temporary files
clean:
	-rm *.o
	-rm $(BIN)

%.o: %.asm
	$(NASM) $(ASM_FLAGS) -o $@ $<
	
%.o: %.c
	$(CC) $(C_FLAGS) -o $@ $<
