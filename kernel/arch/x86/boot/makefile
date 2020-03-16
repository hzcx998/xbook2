# MIT License
# Copyright (c) 2020 Jason Hu, Zhu Yu

BOOT_BIN = boot.bin
LOAD_BIN = loader.bin

PREPARE	+= $(BOOT_BIN) $(LOAD_BIN)

$(BOOT_BIN): boot.asm
	@echo [NASM]  $(patsubst $(TOPDIR)/%,%,$(CURDIR))/$@
	@$(NASM) -o $@ $<
$(LOAD_BIN): loader.asm
	@echo [NASM]  $(patsubst $(TOPDIR)/%,%,$(CURDIR))/$@
	@$(NASM) -o $@ $<

xclean:
	$(RM) $(BOOT_BIN) $(LOAD_BIN)
