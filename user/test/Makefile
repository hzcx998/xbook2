X_LIBS		+= xlibc-0.0.1.a sgi-0.0.1.a pthread-0.0.1.a

NAME		:= test
SRC			+= main.c cond_sync.c

define CUSTOM_TARGET_CMD
echo [APP] $@; \
$(LD) $(X_LDFLAGS) $(X_OBJS) -o $@ $(patsubst %, -L%, $(X_LIBDIRS)) --start-group $(patsubst %, -l:%, $(X_LIBS)) --end-group; \
cp $@ $(srctree)/../develop/rom/bin
endef