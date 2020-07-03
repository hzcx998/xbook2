# makefile for test

#The tools name
NASM 		= nasm
CC			= gcc
LD 			= ld
DD 			= dd
OBJDUMP		= objdump
COPY		= cp

#The file path
DIR_LIBARY = ../../libary

# libarys
DIR_XLIBC 		= $(DIR_LIBARY)/xlibc
DIR_SGI 		= $(DIR_LIBARY)/sgi
DIR_PTHREAD 	= $(DIR_LIBARY)/pthread

#rom dir
DIR_ROM = ../../develop/rom

BIN = $(DIR_ROM)/bin/test
BIN_DIS = out.disasm

#flags
ASM_FLAGS	= -f elf \
			-I $(DIR_XLIBC)/include 

C_FLAGS		= -c -fno-builtin -Wall -Wunused -fno-PIE -m32 -fno-stack-protector \
			-I $(DIR_XLIBC)/include \
			-I $(DIR_SGI)/include \
			-I $(DIR_PTHREAD)/include \
			

LDFLAGS		= -no-pie -m elf_i386 -e _start -Ttext 0x00001000

OBJS =  main.o \
		cond_sync.o \


# 被依赖库应放到后面。
LIB_FILE := $(DIR_SGI)/sgi-0.0.1.a \
			$(DIR_PTHREAD)/pthread-0.0.1.a \
			$(DIR_XLIBC)/xlibc-0.0.1.a \
			

#First read here
.PHONY: all dis

all: compile link

#Compile files
compile: $(OBJS)

link: $(BIN)
$(BIN): $(OBJS) $(LIB_FILE)
	$(LD) $(LDFLAGS) -o $(BIN) $(OBJS) $(LIB_FILE)


dis: $(BIN)
	$(OBJDUMP) -M intel -D $(BIN) > $(BIN_DIS)

#Clean temporary files
clean:
	-rm *.o
	-rm $(BIN)

%.o: %.asm
	$(NASM) $(ASM_FLAGS) -o $@ $<
	
%.o: %.c
	$(CC) $(C_FLAGS) -o $@ $<
