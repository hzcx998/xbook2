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

# libc select
ifeq ($(ENV_LIBC),xlibc)
X_INCDIRS	:= 	$(LIBS_DIR)/xlibc/include
X_LIBDIRS	:= 	$(LIBS_DIR)/xcrt \
				$(LIBS_DIR)/xlibc
else ifeq ($(ENV_LIBC),tinylibc)
X_INCDIRS	:= 	$(LIBS_DIR)/tinylibc/include
X_LIBDIRS	:= 	$(LIBS_DIR)/tinylibc
else ifeq ($(ENV_LIBC),musl)
X_INCDIRS	:= 	$(LIBS_DIR)/musl/include
X_LIBDIRS	:= 	$(LIBS_DIR)/musl/lib
endif

endif

ifeq ($(ENV_LIBC),xlibc)
X_LIBS		:= libxcrt.a libxlibc.a libgcc.a 
else ifeq ($(ENV_LIBC),tinylibc)
X_LIBS		:= libtinylibc.a
else ifeq ($(ENV_LIBC),musl)
X_LIBS		:= crt1.o crti.o crtn.o libc.a libcrypt.a libdl.a libm.a libpthread.a
X_LIBS		+= libresolv.a librt.a libutil.a libxnet.a
X_LIBS		+= libgcc.a
endif
LD			:=  $(ENV_LD)
AS			:=	$(ENV_AS)

export AS LD

ifeq ($(ENV_ARCH), x86)
MODULE      +=  init
MODULE      +=  login
MODULE      +=  netserv
else ifeq ($(ENV_ARCH),riscv64)
ifeq ($(ENV_LIBC),xlibc)
MODULE      +=  init
else ifeq ($(ENV_LIBC),tinylibc)
MODULE      +=  init
else ifeq ($(ENV_LIBC),musl)
MODULE      +=  init
endif
endif