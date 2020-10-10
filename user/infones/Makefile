
X_INCDIRS	+= infones/core
X_LIBS		+= libxlibc-0.0.1.a libgapi-0.1.0.a
NAME		:= infones
SRC			+= infoNES_System_Book.c

define CUSTOM_TARGET_CMD
echo [APP] $@; \
$(LD) $(X_LDFLAGS) $(X_OBJS) -o $@ $(patsubst %, -L%, $(X_LIBDIRS)) --start-group $(patsubst %, -l:%, $(X_LIBS)) --end-group; \
cp $@ $(srctree)/../develop/rom/bin
endef
