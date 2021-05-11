include $(XBUILD_DIR)/include.mk

quiet_cmd_cc_o_c = $(ECHO_CC) $(@:.o=)
cmd_cc_o_c = $(CC) $(X_CFLAGS) -MD -MF $(@D)/.$(@F).d $(X_CPPFLAGS) -c $< -o $@

quiet_cmd_as_o_S = $(ECHO_AS) $(@:.o=)
cmd_as_o_S = $(CC) $(X_CFLAGS) -MD -MF $(@D)/.$(@F).d $(X_CPPFLAGS) -c $< -o $@

quiet_cmd_as_o_asm = $(ECHO_AS) $(@:.o=)
cmd_as_o_asm = $(AS) $(X_ASFLAGS) -o $@  $<

quiet_cmd_cc_o_cpp = $(ECHO_CXX) $(@:.o=)
cmd_cc_o_cpp = $(CXX) $(X_CXXFLAGS) -MD -MF $(@D)/.$(@F).d $(X_CPPFLAGS) -c $< -o $@

# If the list of objects to link is empty, just create an empty built-in.o
quiet_cmd_link_o_target = $(ECHO_LD) $(obj)/built-in.o
cmd_link_o_target = $(if $(strip $(X_OBJS)), \
		      $(LD) -r -o $@ $(filter $(X_OBJS), $^), \
		      $(RM) $@;$(AR) rcs $@)

$(obj)/built-in.o : $(X_OBJS) FORCE
	$(call if_changed,link_o_target)

$(obj)/%.S.o : $(src)/%.S FORCE
	$(call if_changed_dep,as_o_S)

$(obj)/%.asm.o : $(src)/%.asm FORCE
	$(call if_changed,as_o_asm)

$(obj)/%.c.o : $(src)/%.c FORCE
	$(call if_changed_dep,cc_o_c)

$(obj)/%.cpp.o : $(src)/%.cpp FORCE
	$(call if_changed_dep,cc_o_cpp)

# For module target
ifneq ($(NAME),)
ifeq ($(origin CUSTOM_TARGET_CMD),undefined)
quiet_cmd_link_o_binary = $(ECHO_OUTPUT) $(obj)/$(NAME)$(SUFFIX)
cmd_link_o_binary= $(CC) $(X_CFLAGS) $(X_CPPFLAGS) $(X_OBJS) -o $@ $(X_LDFLAGS) $(X_LDLIBS)

quiet_cmd_ar_o_static = $(ECHO_AR) $(obj)/lib$(NAME).a
cmd_ar_o_static = $(if $(strip $(X_OBJS)), \
		      $(AR) rc $@ $(filter $(X_OBJS), $^), \
		      $(RM) $@;$(AR) rcs $@)

quiet_cmd_link_o_shared = $(ECHO_LD) $(obj)/lib$(NAME).$(SHARED_SUFFIX)
cmd_link_o_shared = $(CC) -shared $(X_CFLAGS) $(X_CPPFLAGS) $(X_OBJS) -o $@ $(X_LDFLAGS)  $(X_LDLIBS)

$(obj)/$(NAME)$(SUFFIX) : $(X_OBJS) FORCE
	$(call if_changed,link_o_binary)

$(obj)/lib$(NAME).a : $(X_OBJS) FORCE
	$(call if_changed,ar_o_static)

$(obj)/lib$(NAME).$(SHARED_SUFFIX) : $(X_OBJS) FORCE
	$(call if_changed,link_o_shared)
else
quiet_cmd_link_o_custom =
cmd_link_o_custom = $(CUSTOM_TARGET_CMD)

$(obj)/$(NAME) : $(X_OBJS) FORCE
	$(call if_changed,link_o_custom)
endif
endif