X_LIBS		+= libxlibc-0.0.1.a

X_LDFLAGS	:= -no-pie -m elf_i386 -e _start -Ttext 0x00001000 -s

NAME		:= des
SRC			+= main.c

define CUSTOM_TARGET_CMD
echo [APP] $@; \
$(LD) $(X_LDFLAGS) $(X_OBJS) -o $@ $(patsubst %, -L%, $(X_LIBDIRS)) --start-group $(patsubst %, -l:%, $(X_LIBS)) --end-group; \
cp $@ $(srctree)/../develop/rom/bin; \
rm $@
endef
