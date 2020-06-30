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

# include path
DIR_INCLUDE = ./include

# lwip
DIR_LWIP 	= ./lwip

# lwip
DIR_LWIP_INCLUDE 	= $(DIR_LWIP)/include

# dir
LWIP_IPV4_INCLUDE 	= $(DIR_LWIP_INCLUDE)/ipv4
LWIP_NETIF_INCLUDE 	= $(DIR_LWIP_INCLUDE)/netif
LWIP_ARCH_INCLUDE 	= $(DIR_LWIP_INCLUDE)/arch
LWIP_ARCH_DIR		= $(DIR_LWIP)/arch
LWIP_CORE_DIR		= $(DIR_LWIP)/core
LWIP_NETIF_DIR		= $(DIR_LWIP)/netif
LWIP_API_DIR		= $(DIR_LWIP)/api
LWIP_IPV4_DIR		= $(LWIP_CORE_DIR)/ipv4
LWIP_SNMP_DIR		= $(LWIP_CORE_DIR)/snmp


# rom dir
DIR_ROM = ../../develop/rom

# libarys
DIR_XLIBC 		= $(DIR_LIBARY)/xlibc
DIR_PTHREAD 	= $(DIR_LIBARY)/pthread

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
			-I $(DIR_XLIBC)/include \
			-I $(DIR_PTHREAD)/include \
			
C_FLAGS		= -c -fno-builtin -Wall -Wunused -m32 \
			-I $(DIR_INCLUDE) \
			-I $(DIR_LWIP_INCLUDE) \
			-I $(LWIP_IPV4_INCLUDE) \
			-I $(LWIP_NETIF_INCLUDE) \
			-I $(LWIP_ARCH_INCLUDE) \
			-I $(DIR_XLIBC)/include \
			-I $(DIR_PTHREAD)/include \


# object files

LWIP_ARCH_OBJS = 	$(LWIP_ARCH_DIR)/sys_arch.o \

LWIP_CORE_OBJS = 	$(LWIP_CORE_DIR)/mem.o \
					$(LWIP_CORE_DIR)/memp.o \
					$(LWIP_CORE_DIR)/def.o \
					$(LWIP_CORE_DIR)/dhcp.o \
					$(LWIP_CORE_DIR)/dns.o \
					$(LWIP_CORE_DIR)/init.o \
					$(LWIP_CORE_DIR)/netif.o \
					$(LWIP_CORE_DIR)/pbuf.o \
					$(LWIP_CORE_DIR)/raw.o \
					$(LWIP_CORE_DIR)/stats.o \
					$(LWIP_CORE_DIR)/sys.o \
					$(LWIP_CORE_DIR)/tcp_in.o \
					$(LWIP_CORE_DIR)/tcp_out.o \
					$(LWIP_CORE_DIR)/tcp.o \
					$(LWIP_CORE_DIR)/timers.o \
					$(LWIP_CORE_DIR)/udp.o \
	
LWIP_IPV4_OBJS = 	$(LWIP_IPV4_DIR)/autoip.o \
					$(LWIP_IPV4_DIR)/icmp.o \
					$(LWIP_IPV4_DIR)/igmp.o \
					$(LWIP_IPV4_DIR)/inet_chksum.o \
					$(LWIP_IPV4_DIR)/inet.o \
					$(LWIP_IPV4_DIR)/ip_addr.o \
					$(LWIP_IPV4_DIR)/ip_frag.o \
					$(LWIP_IPV4_DIR)/ip.o \
					
LWIP_SNMP_OBJS = 	$(LWIP_SNMP_DIR)/asn1_dec.o \
					$(LWIP_SNMP_DIR)/asn1_enc.o \
					$(LWIP_SNMP_DIR)/mib_structs.o \
					$(LWIP_SNMP_DIR)/mib2.o \
					$(LWIP_SNMP_DIR)/msg_in.o \
					$(LWIP_SNMP_DIR)/msg_out.o \

LWIP_NETIF_OBJS = 	$(LWIP_NETIF_DIR)/etharp.o \
					$(LWIP_NETIF_DIR)/ethernetif.o \

LWIP_API_OBJS =  	$(LWIP_API_DIR)/api_lib.o \
					$(LWIP_API_DIR)/api_msg.o \
					$(LWIP_API_DIR)/err.o \
					$(LWIP_API_DIR)/netbuf.o \
					$(LWIP_API_DIR)/netdb.o \
					$(LWIP_API_DIR)/netifapi.o \
					$(LWIP_API_DIR)/sockets.o \
					$(LWIP_API_DIR)/tcpip.o \


# objects
LWIP_OBJS	:=  $(LWIP_CORE_OBJS) $(LWIP_ARCH_OBJS) $(LWIP_IPV4_OBJS) \
				$(LWIP_SNMP_OBJS) $(LWIP_NETIF_OBJS)  $(LWIP_API_OBJS)

CORE_OBJS	=

DRIVERS_OBJS	= drivers/netcard.o

MAIN_OBJS 	= 	main.o \

OBJS =  $(MAIN_OBJS) $(LWIP_OBJS) $(DRIVERS_OBJS)

LIB_FILE 	:=  $(DIR_PTHREAD)/pthread-0.0.1.a \
				$(DIR_XLIBC)/xlibc-0.0.1.a \

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
