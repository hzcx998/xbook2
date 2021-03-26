all:
# Don't include env.mk again
include_Makefile_env:=1
export include_Makefile_env

include $(XBUILD_DIR)/define.mk
include $(XBUILD_DIR)/include.mk

srctree		:= $(BUILD_SRC)
objtree		:= $(CURDIR)
src			:= .
obj			:= .

VPATH		:= $(srctree)

export srctree objtree VPATH

ROOT_DIR	:= .
CP_FIXDEP	:= 1
# config
X_CONF_DIR	:=	$(obj)/include/config
X_CONF_FILE	:=	$(srctree)/include/xconfigs.h

# compiler's flags
X_ASFLAGS	:=
X_CFLAGS	:=
X_CXXFLAGS	:=
X_LDFLAGS	:=
X_LIBDIRS	:=
X_LIBS		:=
X_DEFINES	:=
X_INCDIRS	:=
X_INCS		:=
X_CPPFLAGS	:=

ifneq ($(wildcard $(X_CONF_FILE)),)
X_CPPFLAGS	+=	-include $(X_CONF_DIR)/autoconf.h
endif

export X_ASFLAGS X_CFLAGS X_CXXFLAGS X_LDFLAGS X_LIBDIRS X_LIBS X_DEFINES X_LDFLAGS X_INCDIRS X_INCS
export BUILD_OBJ BUILD_SRC X_CONF_DIR

PHONY	+=	all clean xbegin xend xclean conf fixdep $(ROOT_DIR)

ifneq ($(wildcard $(X_CONF_FILE)),)
include $(XBUILD_DIR)/conf.mk

xbegin: conf
endif

xbegin: $(objtree)/fixdep$(SUFFIX)
all: xend
xend: $(ROOT_DIR)

$(objtree)/fixdep$(SUFFIX): $(XBUILD_DIR)/fixdep.c
	@$(ECHO) '$(ECHO_HOSTCC)' fixdep.c
ifeq ($(strip $(HOSTOS)),windows)
ifeq ($(strip $(CP_FIXDEP)),0)
	@$(HOSTCC) -o $@ $< -lwsock32
else
	@$(CP) $(XBUILD_DIR)/fixdep$(SUFFIX) $(objtree)/fixdep$(SUFFIX)
endif
else
	@$(HOSTCC) -o $@ $<
endif

ifneq ($(MAKECMDGOALS),clean)
$(ROOT_DIR): xbegin
endif

$(ROOT_DIR):
	@$(MAKE) $(build)=$@ ISMODULE=1 $(MAKECMDGOALS)

clean: $(ROOT_DIR)
ifneq ($(wildcard $(objtree)/fixdep$(SUFFIX)),)
	@$(ECHO) '$(ECHO_RM)' fixdep
	@$(RM) $(objtree)/fixdep$(SUFFIX)
endif
ifneq ($(wildcard $(X_CONF_DIR)),)
	@$(ECHO) '$(ECHO_RM)' $(X_CONF_DIR)
	@$(RM) $(X_CONF_DIR)
endif

PHONY += FORCE

FORCE:

# Declare the contents of the .PHONY variable as phony.  We keep that
# information in a variable so we can use it in if_changed and friends.
.PHONY: $(PHONY)