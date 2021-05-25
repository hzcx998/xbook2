sinclude ../scripts/env.mk
sinclude ../scripts/localenv.mk

LIBS_DIR	:= ../libs

X_ASFLAGS	:= $(ENV_AFLAGS)
X_CFLAGS	:= $(ENV_CFLAGS)

ifeq ($(ENV_ARCH), x86)
X_INCDIRS	:= $(LIBS_DIR)/xlibc/include \
				$(LIBS_DIR)/pthread/include \
				$(LIBS_DIR)/uview/include \

X_LDFLAGS	:= $(ENV_LDFLAGS) $(ENV_USER_LD_SCRIPT)
X_LIBDIRS	+= 	$(LIBS_DIR)/xlibc \
				$(LIBS_DIR)/pthread \
				$(LIBS_DIR)/uview \
				$(LIBS_DIR)/xcrt

else ifeq ($(ENV_ARCH), riscv64)
X_INCDIRS	:= 
X_LDFLAGS	:= $(ENV_LDFLAGS) $(ENV_USER_LD_SCRIPT)
X_INCDIRS	:= 	$(LIBS_DIR)/xlibc/include

X_LIBDIRS	+= 	$(LIBS_DIR)/xcrt \
				$(LIBS_DIR)/xlibc

endif 

X_LIBS		:= libxcrt.a

LD			:=  $(ENV_LD)
AS			:=	$(ENV_AS)

export AS LD

ifeq ($(ENV_ARCH), x86)
MODULE      +=  init
MODULE      +=  login
MODULE      +=  netserv
else ifeq ($(ENV_ARCH), riscv64)
MODULE      +=  init
#MODULE      +=  utest
#MODULE      +=  test1
endif 