# init for graph mode

X_LIBS		+= libxlibc-0.0.1.a

X_INCDIRS	+= init/include init/fatfs

NAME		:= initg
SRC			+= main.c 

define CUSTOM_TARGET_CMD
echo [APP] $@; \
$(LD) $(X_LDFLAGS) $(X_OBJS) -o $@ $(patsubst %, -L%, $(X_LIBDIRS)) --start-group $(patsubst %, -l:%, $(X_LIBS)) --end-group; \
cp $@ $(srctree)/../develop/rom/sbin
endef
