X_LIBS		+= libxlibc-0.0.1.a libgapi-0.1.0.a libpng-1.6.37.a libzlib-1.2.11.a libjpeg-9d.a

NAME		:= vt100
SRC			+= main.c vt100.c console.c test.c

define CUSTOM_TARGET_CMD
echo [APP] $@; \
$(LD) $(X_LDFLAGS) $(X_OBJS) -o $@ $(patsubst %, -L%, $(X_LIBDIRS)) --start-group $(patsubst %, -l:%, $(X_LIBS)) --end-group; \
cp $@ $(srctree)/../develop/rom/bin
endef
