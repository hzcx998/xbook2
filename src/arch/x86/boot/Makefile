# MIT License
# Copyright (c) 2020 Jason Hu, Zhu Yu

BOOT_BIN = boot.bin
LOAD_BIN = loader.bin
SETUP_BIN = setup.bin

PREPARE	+= $(BOOT_BIN) $(LOAD_BIN) $(SETUP_BIN)

SETUP_OBJS = start.o setup.o lib.o page.o

$(BOOT_BIN): boot.asm
	@echo [NASM]  $(patsubst $(TOPDIR)/%,%,$(CURDIR))/$@
	@$(NASM) -o $@ $<

$(LOAD_BIN): loader.asm
	@echo [NASM]  $(patsubst $(TOPDIR)/%,%,$(CURDIR))/$@
	@$(NASM) -o $@ $<

# make setup elf
setup.elf :  $(SETUP_OBJS)
	@echo [LD]  $(patsubst $(TOPDIR)/%,%,$(CURDIR))/$@
	@$(LD) $(LDFLAGS) -no-pie -e _start -Ttext 0x91000 -o setup.elf $(SETUP_OBJS)  

#$(SETUP_BIN) : setup.asm
#	@echo [NASM]  $(patsubst $(TOPDIR)/%,%,$(CURDIR))/$@
#	@$(NASM) -o $@ $<
$(SETUP_BIN) : setup.elf
	@echo [OBJCOPY]  $(patsubst $(TOPDIR)/%,%,$(CURDIR))/$@
	@$(OBJCOPY) $(OBJCOPY_FLAGS) -O binary setup.elf $(SETUP_BIN)

%.o : %.c
	@echo [CC]  $(patsubst $(TOPDIR)/%,%,$(CURDIR))/$@
	@$(CC) $(CFLAGS) -o $@ $<

start.o : start.asm
	@echo [NASM]  $(patsubst $(TOPDIR)/%,%,$(CURDIR))/$@
	@$(NASM) -f elf -o $@ $<

xclean:
	$(RM) $(BOOT_BIN) $(LOAD_BIN) $(SETUP_BIN) setup.elf $(SETUP_OBJS)
