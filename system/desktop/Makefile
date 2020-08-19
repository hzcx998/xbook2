X_LIBS		+= libxlibc-0.0.1.a libjpeg-9d.a

X_INCDIRS	+= ../library/jpeg ../library/xlibc

NAME		:= desktop
SRC			+= main.c desktop.c taskbar.c window.c

define CUSTOM_TARGET_CMD
echo [APP] $@; \
$(LD) $(X_LDFLAGS) $(X_OBJS) -o $@ $(patsubst %, -L%, $(X_LIBDIRS)) --start-group $(patsubst %, -l:%, $(X_LIBS)) --end-group; \
cp $@ $(srctree)/../develop/rom/sbin; \
rm $@
endef
