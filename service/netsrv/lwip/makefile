# tools
AR 			= ar
NASM 		= nasm
CC			= gcc
RM			= rm

# dir
DIR_INCLUDE 	= ./include
IPV4_INCLUDE 	= $(DIR_INCLUDE)/ipv4
NETIF_INCLUDE 	= $(DIR_INCLUDE)/netif
ARCH_DIR		= ./arch
CORE_DIR		= ./core
NETIF_DIR		= ./netif
API_DIR			= ./api
IPV4_DIR		= $(CORE_DIR)/ipv4
SNMP_DIR		= $(CORE_DIR)/snmp

XBOOKLIB_INCLUDE 	= ../xbooklib/include

# flags
ARFLAGS := -cr
C_FLAGS	:=  -c -fno-builtin -nostdinc -Wall -Wunused -fno-PIE -Wunused -fno-stack-protector -m32 \
			-fno-stack-protector -std=c99 \
			-I $(DIR_INCLUDE) \
			-I $(XBOOKLIB_INCLUDE) \
			-I $(XBOOKLIB_INCLUDE)/pthread \
			-I $(IPV4_INCLUDE) \
			-I $(NETIF_INCLUDE) \
			
NASM_FLAGS	:= -f elf \
			-I $(DIR_INCLUDE) \
			-I $(XBOOKLIB_INCLUDE) \
			-I $(XBOOKLIB_INCLUDE)/pthread \
			-I $(IPV4_INCLUDE) \
			-I $(NETIF_INCLUDE) \

ARCH_OBJS = 	$(ARCH_DIR)/sys_arch.o \

CORE_OBJS = 	$(CORE_DIR)/mem.o \
				$(CORE_DIR)/memp.o \
				$(CORE_DIR)/def.o \
				$(CORE_DIR)/dhcp.o \
				$(CORE_DIR)/dns.o \
				$(CORE_DIR)/init.o \
				$(CORE_DIR)/netif.o \
				$(CORE_DIR)/pbuf.o \
				$(CORE_DIR)/raw.o \
				$(CORE_DIR)/stats.o \
				$(CORE_DIR)/sys.o \
				$(CORE_DIR)/tcp_in.o \
				$(CORE_DIR)/tcp_out.o \
				$(CORE_DIR)/tcp.o \
				$(CORE_DIR)/timers.o \
				$(CORE_DIR)/udp.o \
	
IPV4_OBJS = 	$(IPV4_DIR)/autoip.o \
				$(IPV4_DIR)/icmp.o \
				$(IPV4_DIR)/igmp.o \
				$(IPV4_DIR)/inet_chksum.o \
				$(IPV4_DIR)/inet.o \
				$(IPV4_DIR)/ip_addr.o \
				$(IPV4_DIR)/ip_frag.o \
				$(IPV4_DIR)/ip.o \
				
SNMP_OBJS = 	$(SNMP_DIR)/asn1_dec.o \
				$(SNMP_DIR)/asn1_enc.o \
				$(SNMP_DIR)/mib_structs.o \
				$(SNMP_DIR)/mib2.o \
				$(SNMP_DIR)/msg_in.o \
				$(SNMP_DIR)/msg_out.o \

NETIF_OBJS = 	$(NETIF_DIR)/etharp.o \
				$(NETIF_DIR)/ethernetif.o \

API_OBJS =  	$(API_DIR)/api_lib.o \
				$(API_DIR)/api_msg.o \
				$(API_DIR)/err.o \
				$(API_DIR)/netbuf.o \
				$(API_DIR)/netdb.o \
				$(API_DIR)/netifapi.o \
				$(API_DIR)/sockets.o \
				$(API_DIR)/tcpip.o \
# objects
OBJS	:=  $(CORE_OBJS) $(ARCH_OBJS) $(IPV4_OBJS) $(SNMP_OBJS) $(NETIF_OBJS)  $(API_OBJS)

# out file
LIBNAME = lwip-1.4.1.a
	
# all need to do
all: $(LIBNAME) 

# make a library file
$(LIBNAME): $(OBJS) 
	@echo [AR] making library file $(LIBNAME)
	@$(AR) $(ARFLAGS) $(LIBNAME) $(OBJS)

%.o : %.c
	@echo [CC] $@ $<
	@$(CC) $(C_FLAGS) -o $@ $<

%.o : %.asm
	@echo [NASM] $@ $<
	@$(NASM) $(NASM_FLAGS) -o $@ $<

# clean .o .a files
clean: 
	-$(RM) $(LIBNAME)
	-$(RM) $(OBJS)
	