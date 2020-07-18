X_ASFLAGS	:= -f elf -I xlibc/include
X_INCDIRS	:= xlibc/include jpeg/

SRC			:= *.c

TARGET_TYPE	:= static
NAME		:= jpeg-9d.a

define CUSTOM_TARGET_CMD
echo [LIB] $@; \
$(AR) -rc  $@ $(X_OBJS)
endef