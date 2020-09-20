X_LIBS		+= libxlibc-0.0.1.a libpthread-0.0.1.a libjpeg-9d.a libzlib-1.2.11.a libpng-1.6.37.a

X_CFLAGS    := -fno-builtin -Wall -Wunused -fno-PIE -m32 -fno-stack-protector -std=gnu99 -O2

NAME		:= tests
SRC			+= main.c test_png.c test_jpg.c libvideo.c
SRC			+= socket_test.c
SRC			+= pipe_test.c
SRC			+= shm_test.c
SRC			+= trig_test.c
SRC			+= xlibc_test.c
SRC			+= math_test.c
SRC			+= http_test.c
SRC			+= getopt_test.c
SRC			+= buddy_test.c


define CUSTOM_TARGET_CMD
echo [APP] $@; \
$(LD) $(X_LDFLAGS) $(X_OBJS) -o $@ $(patsubst %, -L%, $(X_LIBDIRS)) --start-group $(patsubst %, -l:%, $(X_LIBS)) --end-group; \
cp $@ $(srctree)/../develop/rom/bin; \
rm $@
endef
