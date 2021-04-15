sinclude ../scripts/env.mk
sinclude ../scripts/localenv.mk

LIBS_DIR	:= ../libs

X_ASFLAGS	:= $(ENV_AFLAGS)
X_CFLAGS	:= $(ENV_CFLAGS)

X_INCDIRS	:= $(LIBS_DIR)/xlibc/include \
				$(LIBS_DIR)/pthread/include \
				$(LIBS_DIR)/netsocket/include \
				$(LIBS_DIR)/uview/include \
	
X_LDFLAGS	:= $(ENV_LDFLAGS) $(ENV_USER_LD_SCRIPT)
X_LIBDIRS	+= $(LIBS_DIR)/xlibc \
				$(LIBS_DIR)/netsocket \
				$(LIBS_DIR)/pthread \
				$(LIBS_DIR)/uview \
				$(LIBS_DIR)/xcrt \

X_LIBS		:= libxcrt.a

LD			:=  $(ENV_LD)
AS			:=	$(ENV_AS)

export AS LD

MODULE      +=  init
MODULE      +=  login
MODULE      +=  netserv
