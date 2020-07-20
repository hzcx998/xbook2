X_LIBS		+= libxlibc-0.0.1.a libpthread-0.0.1.a

X_INCDIRS	+= netsrv/include/ netsrv/lwip/include/ netsrv/lwip/include/ipv4/

NAME		:= netsrv
SRC			+= main.c core/*.c drivers/*.c
SRC			+= lwip/api/*.c lwip/arch/*.c lwip/core/*.c
SRC			+= lwip/core/ipv4/*.c lwip/core/snmp/*.c lwip/netif/*.c

define CUSTOM_TARGET_CMD
echo [APP] $@ ; \
$(LD) $(X_LDFLAGS) $(X_OBJS) -o $@ $(patsubst %, -L%, $(X_LIBDIRS)) --start-group $(patsubst %, -l:%, $(X_LIBS)) --end-group; \
cp $@ $(srctree)/../develop/rom/sbin
endef
