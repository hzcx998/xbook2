sinclude ../scripts/env.mk
sinclude ../scripts/localenv.mk

LIBS_DIR	:= ../libs

X_ASFLAGS	:= $(ENV_AFLAGS)
X_CFLAGS	:= $(ENV_CFLAGS)

ifeq ($(ENV_ARCH), x86)
X_INCDIRS	:= $(LIBS_DIR)/xlibc/include $(LIBS_DIR)/pthread/include \
				$(LIBS_DIR)/uview/include \
				$(LIBS_DIR)/sconf \

X_LDFLAGS	:= $(ENV_LDFLAGS) $(ENV_USER_LD_SCRIPT)
X_LIBDIRS	:= $(LIBS_DIR)/xlibc $(LIBS_DIR)/pthread \
				$(LIBS_DIR)/uview \
				$(LIBS_DIR)/sconf \
				$(LIBS_DIR)/xcrt
				
else ifeq ($(ENV_ARCH), riscv64)
X_LDFLAGS	:= $(ENV_LDFLAGS) $(ENV_USER_LD_SCRIPT)

# libc select
ifeq ($(ENV_LIBC),xlibc)
X_INCDIRS	:= 	$(LIBS_DIR)/xlibc/include
X_LIBDIRS	:= 	$(LIBS_DIR)/xcrt \
				$(LIBS_DIR)/xlibc
else ifeq ($(ENV_LIBC),tinylibc)
X_INCDIRS	:= 	$(LIBS_DIR)/tinylibc/include
X_LIBDIRS	:= 	$(LIBS_DIR)/tinylibc
endif

X_INCDIRS	+=	$(LIBS_DIR)/sconf
X_LIBDIRS += $(LIBS_DIR)/sconf

endif 

ifeq ($(ENV_LIBC),xlibc)
X_LIBS		:= libxcrt.a libxlibc.a
else ifeq ($(ENV_LIBC),tinylibc)
X_LIBS		:= libtinylibc.a
endif

AS			:=	$(ENV_AS)
LD			:=  $(ENV_LD)

export AS LD

ifeq ($(ENV_ARCH), x86)
# bin
MODULE      +=  bash
MODULE      +=  uview
MODULE      +=  echo
MODULE      +=  cat
MODULE      +=  cat2
MODULE      +=  cal
MODULE      +=  cpuinfo
MODULE      +=  ls
MODULE      +=  cp
MODULE      +=  rm
MODULE      +=  ps
MODULE      +=  mem
MODULE      +=  date
MODULE      +=  mv
MODULE      +=  rename
MODULE      +=  mkdir
MODULE      +=  rmdir
MODULE      +=  touch
MODULE      +=  tests
MODULE      +=  hello
MODULE      +=  uname
MODULE      +=  sh
MODULE      +=  wavplayer
MODULE      +=  mp3player
MODULE      +=  grep
MODULE      +=  archfetch
MODULE      +=  mount
MODULE      +=  mkfs
MODULE      +=  unmount
MODULE      +=  losetup
else ifeq ($(ENV_ARCH), riscv64)

ifeq ($(ENV_LIBC),xlibc)
MODULE      +=  sh
MODULE      +=  echo
MODULE      +=  cat
MODULE      +=  cat2
MODULE      +=  cal
MODULE      +=  cpuinfo
MODULE      +=  ls
MODULE      +=  cp
MODULE      +=  rm
MODULE      +=  ps
MODULE      +=  mem
MODULE      +=  date
MODULE      +=  mv
MODULE      +=  rename
MODULE      +=  mkdir
MODULE      +=  rmdir
MODULE      +=  touch
MODULE      +=  tests
MODULE      +=  hello
MODULE      +=  uname
MODULE      +=  archfetch
MODULE      +=  mount
MODULE      +=  mkfs
MODULE      +=  unmount
else ifeq ($(ENV_LIBC),tinylibc)

endif
endif 