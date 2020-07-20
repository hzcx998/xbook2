X_LIBS		+= libxlibc-0.0.1.a libpthread-0.0.1.a

X_INCDIRS	+= guisrv/include/ 

NAME		:= guisrv
SRC			+= main.c console/ drivers/ event/ font/ graph/ input/

define CUSTOM_TARGET_CMD
echo [APP] $@; \
$(LD) $(X_LDFLAGS) $(X_OBJS) -o $@ $(patsubst %, -L%, $(X_LIBDIRS)) --start-group $(patsubst %, -l:%, $(X_LIBS)) --end-group; \
cp $@ $(srctree)/../develop/rom/sbin
endef
