X_LIBS		:= libxlibc.a libpthread.a libxtk.a libpng.a libzlib.a libjpeg.a libgato.a libuview.a

X_CFLAGS	:= -fno-builtin -fno-PIE -m32 -fno-stack-protector -std=gnu99 -O3

NAME		:= gato

SRC			+= main.c

define CUSTOM_TARGET_CMD
echo [APP] $@; \
$(LD) $(X_LDFLAGS) $(X_OBJS) -o $@ $(patsubst %, -L%, $(X_LIBDIRS)) --start-group $(patsubst %, -l:%, $(X_LIBS)) --end-group; \
cp $@ $(srctree)/../../develop/rom/usr
endef