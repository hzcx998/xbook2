X_LIBS		+= xlibc-0.0.1.a

X_INCDIRS	+= filesrv/include filesrv/fatfs

NAME		:= filesrv
SRC			+= core/ drivers/ fatfs/ fsal/ main.c 

define CUSTOM_TARGET_CMD
echo [APP] $@; \
$(LD) $(X_LDFLAGS) $(X_OBJS) -o $@ $(patsubst %, -L%, $(X_LIBDIRS)) --start-group $(patsubst %, -l:%, $(X_LIBS)) --end-group; \
cp $@ $(srctree)/../develop/rom/sbin
endef
