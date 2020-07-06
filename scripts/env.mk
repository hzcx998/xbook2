ifeq ($(include_Makefile_env),)
include_Makefile_env:=1

PHONY		=
all:

# Do not:
# o  use make's built-in rules and variables
#    (this increases performance and avoids hard-to-debug behaviour);
# o  print "Entering directory ...";
MAKEFLAGS	+= -rR -s --no-print-directory

BUILD_SRC	:=
BUILD_OBJ	:=

ifeq ($(BUILD_SRC),)
XBUILD_DIR	:= $(abspath $(lastword $(MAKEFILE_LIST)/../))

include $(XBUILD_DIR)/define.mk

BUILD_OBJ	:= .
ifeq ("$(origin O)", "command line")
BUILD_OBJ	:= $(O)
endif

ifneq ($(BUILD_OBJ),)
PHONY	+=	all $(MAKECMDGOALS) clean

BUILD_OBJ	:= $(abspath $(BUILD_OBJ))

ifneq ($(BUILD_OBJ),$(CURDIR))
ifneq ($(strip $(MAKECMDGOALS)),clean)
__dummy		:= $(shell $(MKDIR) $(BUILD_OBJ))
endif
endif


ifneq ($(__dummy),)
$(error failed to create $(BUILD_OBJ))
endif

$(filter-out all,$(MAKECMDGOALS)) all: sub-mk

sub-mk:
	@$(MAKE) -C $(BUILD_OBJ) XBUILD_DIR=$(XBUILD_DIR) BUILD_SRC=$(CURDIR) -f $(XBUILD_DIR)/wrapper.mk $(MAKECMDGOALS)

ifneq ($(BUILD_OBJ),$(CURDIR))
clean: sub-mk
	@echo [RM] $(BUILD_OBJ)
	@$(RM) $(BUILD_OBJ)
endif

# Declare the contents of the .PHONY variable as phony.  We keep that
# information in a variable so we can use it in if_changed and friends.
.PHONY: $(PHONY)

endif #ifneq ($(BUILD_OBJ),)
endif #ifeq ($(BUILD_SRC),)


endif #ifeq ($(include_Makefile_env),)