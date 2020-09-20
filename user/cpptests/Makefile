X_LIBS		+= libxlibcpp-0.0.1.a libxlibc-0.0.1.a

X_CFLAGS    := -fno-builtin -Wall -fno-PIE -m32 -fno-stack-protector -std=c++98
CC 			:= g++

NAME		:= cpptests

SRC			+= main.c
SRC			+= iostream_test.c

define CUSTOM_TARGET_CMD
echo [APP] $@; \
$(LD) $(X_LDFLAGS) $(X_OBJS) -o $@ $(patsubst %, -L%, $(X_LIBDIRS)) --start-group $(patsubst %, -l:%, $(X_LIBS)) --end-group; \
cp $@ $(srctree)/../develop/rom/bin; \
rm $@
endef
