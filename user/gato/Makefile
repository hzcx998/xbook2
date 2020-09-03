X_LIBS		+= libxlibc-0.0.1.a libpthread-0.0.1.a libgapi-0.1.0.a

X_CFLAGS	:= -fno-builtin -fno-PIE -m32 -fno-stack-protector -std=gnu99 
X_INCDIRS	+= gato/include
NAME		:= gato

SRC			+= examples/ui.c examples/main.c render/*.c image/image.c

define CUSTOM_TARGET_CMD
echo [APP] $@; \
$(LD) $(X_LDFLAGS) $(X_OBJS) -o $@ $(patsubst %, -L%, $(X_LIBDIRS)) --start-group $(patsubst %, -l:%, $(X_LIBS)) --end-group; \
cp $@ $(srctree)/../develop/rom/bin; \
rm $@
endef
