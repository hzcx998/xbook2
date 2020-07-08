PHONY		:=	__build
__build:

src			:=	$(obj)

SRC			:=
INCDIRS		:=
NAME		:=
MODULE		:=
# binary static shared
TARGET_TYPE	:=	binary

X_SUBDIR	:=
X_SUB_OBJ	:=
X_EXTRA		:=
X_PREPARE	:=
X_CLEAN		:=

include $(XBUILD_DIR)/include.mk
sinclude $(X_CONF_DIR)/auto.conf


ifneq ($(wildcard $(srctree)/$(src)/Makefile),)
include $(srctree)/$(src)/Makefile
else
SRC			:=	*.S *.c *.asm
endif # ifneq ($(wildcard $(srctree)/$(src)/Makefile),)

ifeq ($(ISMODULE),0)
X_BUILTIN	:=	$(obj)/built-in.o
endif

X_MODULE	=	$(MODULE)

# X_NAME
ifneq ($(NAME),)
X_NAME		=	$(obj)/$(NAME)
ifeq ($(strip $(TARGET_TYPE)),binary)
X_NAME		=	$(obj)/$(NAME)$(SUFFIX)
else ifeq ($(strip $(TARGET_TYPE)),static)
X_NAME		=	$(obj)/lib$(NAME).a
else ifeq ($(strip $(TARGET_TYPE)),shared)
X_NAME		=	$(obj)/lib$(NAME).$(SHARED_SUFFIX)
else ifeq ($(strip $(filter binary static shared,$(TARGET_TYPE))),)
$(error undefined TARGET_TYPE=$(TARGET_TYPE))
endif
endif

# FLAGS
X_CPPFLAGS	:= $(patsubst %, -I %, $(foreach d,$(X_INCDIRS),$(wildcard $(srctree)/$(d)))) $(patsubst %, -D%, $(X_DEFINES)) $(patsubst %, -include %, $(X_INCS))
X_LDLIBS	:= $(patsubst %, -L%, $(X_LIBDIRS)) $(patsubst %, -l%, $(X_LIBS))

export X_ASFLAGS X_CFLAGS X_LDFLAGS X_LIBDIRS X_LIBS X_DEFINES X_LDFLAGS X_INCDIRS X_INCS

X_CUR_OBJ	:=	$(foreach f,$(filter-out %/, $(SRC)),$(wildcard $(srctree)/$(src)/$(f)))
X_CUR_OBJ	:=	$(patsubst $(srctree)/$(src)/%,$(obj)/%.o,$(X_CUR_OBJ))
X_SUBDIR	:=	$(filter %/,$(foreach f,$(filter %/, $(SRC)),$(wildcard $(srctree)/$(src)/$(f))))
X_SUBDIR	:=	$(patsubst $(srctree)/%/,%,$(X_SUBDIR))
X_SUB_OBJ	:=	$(patsubst $(src)/%,$(obj)/%/built-in.o,$(X_SUBDIR))

X_OBJS		:=	$(X_CUR_OBJ) $(X_SUB_OBJ)
# case: $(obj)==.
X_OBJS		:=	$(patsubst $(objtree)/%,%,$(abspath $(X_OBJS)))
X_TARGET	:=	$(X_BUILTIN) $(X_OBJS) $(X_EXTRA) $(X_NAME)
X_DEPS		:=	$(wildcard $(foreach f,$(X_TARGET),$(dir $(f)).$(notdir $(f)).cmd))

# Add a semicolon to form an empty command and then recheck the dependency
$(sort $(X_SUB_OBJ)) : $(X_SUBDIR) ;
PHONY		+=	$(X_SUBDIR) $(X_MODULE)

$(X_OBJS) $(X_EXTRA) : $(X_PREPARE)
clean: $(X_SUBDIR) $(X_MODULE)

$(X_SUBDIR):
	@$(MAKE) $(build)=$@ ISMODULE=0 $(MAKECMDGOALS)
$(X_MODULE):
	@$(MAKE) $(build)=$(obj)/$@ ISMODULE=1 $(MAKECMDGOALS)

# Create output directory
_dummy		:=	$(shell $(MKDIR) $(obj) $(dir $(X_TARGET)))

PHONY		+=	clean

sinclude $(X_DEPS)

export X_ASFLAGS X_CFLAGS X_CPPFLAGS

__build : $(X_TARGET) $(X_MODULE)
	$(CUSTOM_AFTER_BUILD)

$(X_TARGET): $(X_MODULE)
$(X_NAME): $(X_OBJS)

clean:
ifneq ($(strip $(wildcard $(X_TARGET) $(obj)/.*.cmd $(X_NAME) $(X_CLEAN))),)
	@$(ECHO) '$(ECHO_RM)' $(obj)
	@$(RM) $(X_TARGET) $(wildcard $(obj)/.*.cmd) $(X_NAME) $(X_CLEAN)
endif

include $(XBUILD_DIR)/rule.mk

PHONY += FORCE

FORCE: ;

.PHONY : $(PHONY)
