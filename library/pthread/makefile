X_ASFLAGS	:= -f elf -I pthread/include -I xlibc/include
X_INCDIRS	:= pthread/include xlibc/include

SRC			+= pthread_entry.asm *.c
TARGET_TYPE	:= static
NAME		:= pthread-0.0.1.a

define CUSTOM_TARGET_CMD
echo [LIB] $@; \
$(AR) -rc  $@ $(X_OBJS)
endef