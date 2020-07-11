# makefile for test

#The tools name
NASM 		= nasm
CC			= gcc
LD 			= ld
DD 			= dd
OBJDUMP		= objdump
COPY		= cp

#The file path
DIR_LIBARY = ../../library

DIR_INCLUDE = ./include

# librarys
DIR_XLIBC 		= $(DIR_LIBARY)/xlibc
DIR_SGI 		= $(DIR_LIBARY)/sgi
DIR_PTHREAD 	= $(DIR_LIBARY)/pthread

# rom dir
DIR_ROM = ../../develop/rom

BIN = $(DIR_ROM)/sbin/guisrv
BIN_DIS = a.out.disasm

#flags
ASM_FLAGS	= -f elf \
			-I $(DIR_INCLUDE) \
			-I $(DIR_XLIBC)/include \
			-I $(DIR_SGI)/include \
			-I $(DIR_PTHREAD)/include \

C_FLAGS		= -c -fno-builtin -Wall -Wunused -fno-stack-protector -m32 \
			-I $(DIR_INCLUDE) \
			-I $(DIR_XLIBC)/include \
			-I $(DIR_SGI)/include \
			-I $(DIR_PTHREAD)/include \

DRIVERS_OBJS 	= 	drivers/screen.o \
					drivers/keyboard.o \
					drivers/mouse.o \

FONT_OBJS 		=	font/font.o \
					font/font_simsun.o \
					font/font_standard.o \

INPUT_OBJS 		= 	input/keyboard.o \
					input/mouse.o \
					
GRAPH_OBJS 		= 	graph/text.o \
					graph/rect.o \

CONSOLE_OBJS	= 	console/console.o \
					console/cursor.o \
					console/clipboard.o \

CMD_OBJS		= 	cmd/cmd.o \

EVENT_OBJS		= 	event/event.o \

MAIN_OBJS 		= 	main.o \
					
OBJS 			=  	$(MAIN_OBJS) $(DRIVERS_OBJS) $(FONT_OBJS) $(INPUT_OBJS) \
					$(GRAPH_OBJS) $(CONSOLE_OBJS) $(CMD_OBJS) $(EVENT_OBJS) \
					

LIB_FILE 	:=  $(DIR_PTHREAD)/pthread-0.0.1.a \
				$(DIR_XLIBC)/xlibc-0.0.1.a \
			
# -n: close memory aligin.
#LDFLAGS		= -T link.ld -n
LDFLAGS		= -no-pie -m elf_i386 -e _start -Ttext 0x00001000

#First read here
.PHONY: all dis

all: compile link

#Compile files
compile: $(OBJS)

link: $(BIN) 
$(BIN): $(OBJS) $(LIB_FILE)
	$(LD) $(LDFLAGS) $(OBJS) $(LIB_FILE) -o $(BIN) 


dis: $(BIN)
	$(OBJDUMP) -M intel -D $(BIN) > $(BIN_DIS)

#Clean temporary files
clean:
	-rm $(OBJS)
	-rm $(BIN)

%.o: %.asm
	$(NASM) $(ASM_FLAGS) -o $@ $<
	
%.o: %.c
	$(CC) $(C_FLAGS) -o $@ $<
