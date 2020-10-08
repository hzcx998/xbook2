X_LIBS		+= libxlibcpp-0.0.1.a libxlibc-0.0.1.a

X_CXXFLAGS  := -fno-builtin -Wall -fno-PIE -m32 -fno-stack-protector -std=c++98
CXX 		:= g++

NAME		:= cpptests

SRC			+= main.cpp
SRC			+= iostream_test.cpp

define CUSTOM_TARGET_CMD
echo [APP] $@; \
$(LD) $(X_LDFLAGS) $(X_OBJS) -o $@ $(patsubst %, -L%, $(X_LIBDIRS)) --start-group $(patsubst %, -l:%, $(X_LIBS)) --end-group; \
cp $@ $(srctree)/../develop/rom/bin; \
rm $@
endef
