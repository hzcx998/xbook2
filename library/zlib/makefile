X_ASFLAGS	:= -f elf -I xlibc/include
X_INCDIRS	:= xlibc/include zlib/

SRC			:= *.c

TARGET_TYPE	:= static
NAME		:= zlib-1.2.11.a

define CUSTOM_TARGET_CMD
echo [LIB] $@; \
$(AR) -rc  $@ $(X_OBJS)
endef