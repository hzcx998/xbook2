X_LIBS		+= libxlibc.a
X_INCDIRS	+= grootfs/cpio

NAME		:= grootfs
SRC			+= main.c
SRC			+= cpio/*.c

define CUSTOM_TARGET_CMD
echo [APP] $@; \
$(LD) $(X_LDFLAGS) $(X_OBJS) -o $@ $(patsubst %, -L%, $(X_LIBDIRS)) --start-group $(patsubst %, -l:%, $(X_LIBS)) --end-group; \
cp $@ $(srctree)/../develop/rom/sbin
endef
