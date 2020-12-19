X_LIBS		+= libxlibc-0.0.1.a libpthread-0.0.1.a libnetsocket-0.0.1.a

X_INCDIRS	+= netserv/include netserv/lwip/include netserv/lwip/include/ipv4

NAME		:= netserv
SRC			+= main.c 
SRC			+= netcard.c 
SRC			+= netmain.c 
SRC			+= httpd.c 
SRC			+= netremote.c 
SRC			+= lwip/ 

define CUSTOM_TARGET_CMD
echo [APP] $@; \
$(LD) $(X_LDFLAGS) $(X_OBJS) -o $@ $(patsubst %, -L%, $(X_LIBDIRS)) --start-group $(patsubst %, -l:%, $(X_LIBS)) --end-group; \
cp $@ $(srctree)/../../develop/rom/sbin
endef
