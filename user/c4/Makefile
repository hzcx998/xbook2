X_LIBS		+= libxlibc-0.0.1.a

C_FLAGS		:= -fno-builtin -Wall -Wunused -fno-PIE -m32 -fno-stack-protector -std=gnu99

NAME		:= c4
SRC			+= c4.c

define CUSTOM_TARGET_CMD
echo [APP] $@; \
$(LD) $(X_LDFLAGS) $(X_OBJS) -o $@ $(patsubst %, -L%, $(X_LIBDIRS)) --start-group $(patsubst %, -l:%, $(X_LIBS)) --end-group; \
cp $@ $(srctree)/../develop/rom/bin
endef
