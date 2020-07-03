# makefile for bosh

#The tools name
NASM 		= nasm
CC			= gcc
LD 			= ld
DD 			= dd
OBJDUMP		= objdump

#The file path
DIR_LIBARY = ../../libary

#rom dir
DIR_ROM = ../../develop/rom

# libarys
DIR_XLIBC = $(DIR_LIBARY)/xlibc
DIR_SGI = $(DIR_LIBARY)/sgi

BIN = $(DIR_ROM)/bin/bosh
BIN_DIS = out.disasm

#flags
ASM_FLAGS	= -f elf \
			-I $(DIR_XLIBC)/indlude 

C_FLAGS		= -c -fno-builtin -Wall -Wunused -fno-PIE -m32 -fno-stack-protector \
			-I $(DIR_XLIBC)/include \
			-I $(DIR_SGI)/include 

LDFLAGS		= -m elf_i386 -e _start -Ttext 0x00001000

OBJS =  main.o \
		window.o \
		console.o \
		cmd.o \
		cursor.o \
		clipboard.o \
		
LIB_FILE := $(DIR_SGI)/sgi-0.0.1.a \
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
