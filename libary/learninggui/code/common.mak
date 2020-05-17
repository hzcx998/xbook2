CC_PREFIX   =
CC          = $(CC_PREFIX)gcc
LD          = $(CC_PREFIX)ld
AR          = $(CC_PREFIX)ar
RANLIB      = $(CC_PREFIX)ranlib


CFLAGS      = -std=c89 -fpic -fno-stack-protector -D_POSIX_C_SOURCE=199309L -O1 -Wall
LDFLAGS     = -r


SO_LDFLAGS  = -shared -lrt -lm
A_LDFLAGS   = cqs


VERSION     = 0.5.2


SO_TARGET_S = liblearninggui.so
SO_TARGET   = liblearninggui-$(VERSION).so
A_TARGET_S  = liblearninggui.a
A_TARGET    = liblearninggui-$(VERSION).a
