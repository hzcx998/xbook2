# this called by /src/Makefile
sinclude ../scripts/localenv.mk
ifeq ($(ENV_MACH), mach-qemu)
SRC	+= mach-qemu/
endif
# k210 also use mach-qemu
ifeq ($(ENV_MACH), mach-k210)
SRC	+= mach-qemu/
endif
