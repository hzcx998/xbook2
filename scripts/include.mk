ifeq ($(include_Makefile_include),)
include_Makefile_include:=1

# Convenient variables
comma   := ,
squote  := '
empty   :=
space   := $(empty) $(empty)

###
# Escape single quote for use in echo statements
escsq = $(subst $(squote),'\$(squote)',$1)

echo-cmd = $(if $(quiet_cmd_$(1)),$(ECHO) '$(quiet_cmd_$(1))';)

# >'< substitution is for echo to work,
# >$< substitution to preserve $ when reloading .cmd file
# note: when using inline perl scripts [perl -e '...$$t=1;...']
# in $(cmd_xxx) double $$ your perl vars
make-cmd = $(subst \#,\\\#,$(subst $$,$$$$,$(call escsq,$(cmd_$(1)))))

###
# Shorthand for $(MAKE) -f scripts/build.mk obj=
# Usage:
# $(MAKE) $(build)=dir
build := -f $(XBUILD_DIR)/build.mk obj

# Check if both arguments has same arguments. Result is empty string if equal.
# User may override this check using make KBUILD_NOCMDDEP=1
arg-check = $(strip $(filter-out $(cmd_$(1)), $(cmd_$@)) \
                    $(filter-out $(cmd_$@),   $(cmd_$(1))) )

# Find any prerequisites that is newer than target or that does not exist.
# PHONY targets skipped in both cases.
any-prereq = $(filter-out $(PHONY),$?) $(filter-out $(PHONY) $(wildcard $^),$^)

dot-target	=	$(@D)/.$(@F)
depfile		=	$(dot-target).d

# Execute command if command has changed or prerequisite(s) are updated.
#
if_changed = $(if $(strip $(any-prereq) $(arg-check)), \
	@set -e; \
	$(echo-cmd) $(cmd_$(1)); \
	echo 'cmd_$@ := $(make-cmd)' > $(dot-target).cmd)

fixdep		:=	$(objtree)/fixdep

# Execute the command and also postprocess generated .d dependencies file.
if_changed_dep = $(if $(strip $(any-prereq) $(arg-check)), \
	@set -e; \
	$(echo-cmd) $(cmd_$(1)); \
	$(fixdep) $(depfile) $@ '$(make-cmd)' > $(dot-target).tmp; \
	rm -f $(depfile); \
	mv -f $(dot-target).tmp $(dot-target).cmd)

endif #ifeq ($(include_Makefile_include),)