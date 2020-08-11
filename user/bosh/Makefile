X_LIBS		+= libxlibc-0.0.1.a

NAME		:= bosh
SRC			+= main.c cmd.c shell.c

define CUSTOM_TARGET_CMD
echo [APP] $@; \
$(LD) $(X_LDFLAGS) $(X_OBJS) -o $@ $(patsubst %, -L%, $(X_LIBDIRS)) --start-group $(patsubst %, -l:%, $(X_LIBS)) --end-group; \
cp $@ $(srctree)/../develop/rom/bin; \
rm $@
endef
