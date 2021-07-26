sinclude ../scripts/env.mk
sinclude ../scripts/localenv.mk

X_ASFLAGS	:= $(ENV_AFLAGS)
X_CFLAGS	:= $(ENV_CFLAGS)

X_LDFLAGS	:= $(ENV_LDFLAGS)

LD			:=  $(ENV_LD)
AS			:= $(ENV_AS)
export AS LD

ifeq ($(ENV_LIBC),xlibc)
MODULE		:= xcrt
MODULE		+= xlibc
MODULE		+= sconf
#MODULE		+= xlibcpp
#MODULE		+= pthread
#MODULE		+= uview
else ifeq ($(ENV_LIBC),tinylibc)
MODULE		:= tinylibc
else ifeq ($(ENV_LIBC),musl)
endif