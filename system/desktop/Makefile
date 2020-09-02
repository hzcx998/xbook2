X_LIBS		+= libxlibc-0.0.1.a libjpeg-9d.a libgapi-0.1.0.a libpng-1.6.37.a libzlib-1.2.11.a 

X_INCDIRS	+= desktop 

NAME		:= desktop
SRC			+= main.c desktop.c taskbar.c winctl.c icon.c

define CUSTOM_TARGET_CMD
echo [APP] $@; \
$(LD) $(X_LDFLAGS) $(X_OBJS) -o $@ $(patsubst %, -L%, $(X_LIBDIRS)) --start-group $(patsubst %, -l:%, $(X_LIBS)) --end-group; \
cp $@ $(srctree)/../develop/rom/sbin; \
rm $@
endef
