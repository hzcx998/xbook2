PHONY	+= conf

conf : $(X_CONF_DIR)/auto.conf $(X_CONF_DIR)/autoconf.h

$(X_CONF_DIR)/autoconf.h : FORCE
	$(if $(wildcard $(X_CONF_DIR)),:,@echo [MKDIR] $(X_CONF_DIR) && $(MKDIR) $(X_CONF_DIR))
	$(if $(wildcard $(X_CONF_DIR)/autoconf.h),:,@touch $(X_CONF_DIR)/autoconf.h)
	@$(CC) -E -P -dM $(X_INCDIRS) $(srctree)/$(src)/include/xconfigs.h \
	| sed -n -e "/\#define\s\+CONFIG_[[:alnum:]_]*/p" \
	| sed -n -e "s/\s*$$//p" \
	> $(X_CONF_DIR)/autoconf.temp
	@$(CD) $(X_CONF_DIR) && grep -vxf autoconf.temp autoconf.h \
	| sed -n -e "s/\#define\s\+CONFIG_\([[:alnum:]_]*\).*/\L\1\E.h/p" \
	| xargs touch autoconf.h
	@$(CD) $(X_CONF_DIR) && grep -vxf autoconf.h autoconf.temp \
	| sed -n -e "s/\#define\s\+CONFIG_\([[:alnum:]_]*\).*/\L\1\E.h/p" \
	| xargs touch autoconf.h
	@$(RM) $(X_CONF_DIR)/autoconf.h
	@echo [AUTOCONF]
	@$(MV) $(X_CONF_DIR)/autoconf.temp $(X_CONF_DIR)/autoconf.h

$(X_CONF_DIR)/auto.conf : $(X_CONF_DIR)/autoconf.h
	@$(RM) $(X_CONF_DIR)/auto.conf
	@$(CD) $(X_CONF_DIR) && cat autoconf.h \
	| sed -n -e "s/\#define\s\+\(CONFIG_[[:alnum:]_]*\)[[:space:]]/\1=/p" \
	>> auto.conf
	@$(CD) $(X_CONF_DIR) && cat autoconf.h \
	| sed -n -e "s/\#define\s\+\(CONFIG_[[:alnum:]_]*\)$$/\1=y/p" \
	>> auto.conf
