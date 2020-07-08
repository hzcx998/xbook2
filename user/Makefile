<<<<<<< HEAD
# 文件路径
TEST 		= ./test
INFONES 	= ./infones
BOSH 		= ./bosh
LUA 		= ./lua
=======
sinclude ../scripts/env.mk
>>>>>>> 17d36e39ee27fd9e4c6cc6e7237a014afddb6344

X_ASFLAGS	+= -f elf -I $(DIR_XLIBC)/indlude 
X_CFLAGS    += -fno-builtin -Wall -Wunused -fno-PIE -m32 -fno-stack-protector
X_INCDIRS	+= ../library/xlibc/include ../library/sgi/include ../library/pthread/include 
X_LDFLAGS	+= -no-pie -m elf_i386 -e _start -Ttext 0x00001000
X_LIBDIRS	+= $(srctree)/../library/xlibc $(srctree)/../library/sgi $(srctree)/../library/pthread 

<<<<<<< HEAD
all: 
	$(MAKE) -C $(TEST)
	$(MAKE) -C $(INFONES)
	$(MAKE) -C $(BOSH)
	$(MAKE) -C $(LUA)
	
clean:
	$(MAKE) -C $(TEST) clean
	$(MAKE) -C $(INFONES) clean
	$(MAKE) -C $(BOSH) clean
	$(MAKE) -C $(LUA) clean
=======
AS			:=	nasm

export AS

MODULE      +=  bosh
MODULE      +=  infones
MODULE      +=  test
>>>>>>> 17d36e39ee27fd9e4c6cc6e7237a014afddb6344
