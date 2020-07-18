X_LIBS		+= xlibc-0.0.1.a pthread-0.0.1.a

X_INCDIRS	+= netsrv/include/ netsrv/lwip/include/ netsrv/lwip/include/ipv4/

NAME		:= netsrv
SRC			+= main.c core/ drivers/ lwip/

define CUSTOM_TARGET_CMD
echo [APP] $@ $(srctree)/../develop/rom/sbin; \
$(LD) $(X_LDFLAGS) $(X_OBJS) -o $@ $(patsubst %, -L%, $(X_LIBDIRS)) --start-group $(patsubst %, -l:%, $(X_LIBS)) --end-group; \
cp $@ $(srctree)/../develop/rom/sbin
endef
