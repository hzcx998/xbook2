# makefile for test

#The tools name
NASM 		= nasm
CC			= gcc
LD 			= ld
DD 			= dd
OBJDUMP		= objdump
FATFS 		= ../../tools/fatfs/fatfs

#The file path
DIR_LIBARY = ../../libary

DIR_INCLUDE = ./include

# rom dir
DIR_ROM = ../../develop/rom

DIR_TFTP = ./tftp

DIR_PING = ./ping
DIR_HTTP = ./http

# libarys
DIR_XBOOKLIB 		= $(DIR_LIBARY)/xbooklib
DIR_LWIPLIB 		= $(DIR_LIBARY)/lwiplib

BIN = $(DIR_ROM)/sbin/netsrv
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
			-I $(DIR_INCLUDE) \
			-I $(DIR_LWIPLIB)/include \
			-I $(DIR_LWIPLIB)/include/ipv4 \
			-I $(DIR_LWIPLIB)/include/arch \
			-I $(DIR_XBOOKLIB)/include \
			-I $(DIR_XBOOKLIB)/include/pthread \
			-I $(DIR_PING) \
			
			

C_FLAGS		= -c -fno-builtin -Wall -Wunused -m32 \
			-I $(DIR_INCLUDE) \
			-I $(DIR_LWIPLIB)/include \
			-I $(DIR_LWIPLIB)/include/ipv4 \
			-I $(DIR_LWIPLIB)/include/arch \
			-I $(DIR_XBOOKLIB)/include \
			-I $(DIR_XBOOKLIB)/include/pthread \
			-I $(DIR_PING) \
			
MAIN_OBJS 	= 	main.o \

TFTP_OBJS	=	$(DIR_TFTP)/tftpserver.o \
				$(DIR_TFTP)/tftputils.o \
				$(DIR_TFTP)/lwipdemo.o \
				

PING_OBJS	=	$(DIR_PING)/ping.o \
				$(DIR_PING)/loopback.o \

HTTP_OBJS	=	$(DIR_HTTP)/http.o \


OBJS =  $(MAIN_OBJS) $(PING_OBJS) $(HTTP_OBJS)


LIB_FILE = $(DIR_LWIPLIB)/lwip-1.4.1.a \
			$(DIR_XBOOKLIB)/xbooklib-0.0.1.a \

LDFLAGS		= -m elf_i386 -e _start -Ttext 0x00001000

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
	-rm $(OBJS)
	-rm $(BIN)

%.o: %.asm
	$(NASM) $(ASM_FLAGS) -o $@ $<
	
%.o: %.c
	$(CC) $(C_FLAGS) -o $@ $<
