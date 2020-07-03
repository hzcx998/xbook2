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

DIR_INCLUDE = ./include

# libarys
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

C_FLAGS		= -c -fno-builtin -Wall -Wunused -m32 \
			-I $(DIR_INCLUDE) \
			-I $(DIR_XLIBC)/include \
			-I $(DIR_SGI)/include \
			-I $(DIR_PTHREAD)/include \

DRIVERS_OBJS 	= 	drivers/screen.o \
					drivers/mouse.o \
					drivers/keyboard.o \

LAYER_OBJS 	= 		layer/point.o \
					layer/line.o \
					layer/rect.o \
					layer/bitmap.o \
					layer/layer.o \
					layer/char.o \
					
WINDOW_OBJS 	=	window/window.o \
					window/point.o \
					window/line.o \
					window/rect.o \
					window/bitmap.o \
					window/char.o \
					window/event.o \

ENVIRONMENT_OBJS =	environment/desktop.o \
					environment/winctl.o \
					environment/statusbar.o \
					environment/statusbar-time.o \

FONT_OBJS 		=	font/font.o \
					font/font_simsun.o \
					font/font_standard.o \

WIDGET_OBJS		=	widget/widget.o \
					widget/label.o \
					widget/button.o \

CORE_OBJS 		= 	core/interface.o \
					core/display.o \

INPUT_OBJS 		= 	input/keyboard.o \
					input/mouse.o \

EVENT_OBJS 		= 	event/msgque.o \

MAIN_OBJS 		= 	main.o \

OBJS =  $(MAIN_OBJS) $(DRIVERS_OBJS) $(LAYER_OBJS) $(WINDOW_OBJS) \
		$(FONT_OBJS) $(ENVIRONMENT_OBJS) $(WIDGET_OBJS) $(CORE_OBJS) \
		$(INPUT_OBJS) $(EVENT_OBJS)

LIB_FILE 	:=  $(DIR_PTHREAD)/pthread-0.0.1.a \
				$(DIR_XLIBC)/xlibc-0.0.1.a \
			
# -n: close memory aligin.
#LDFLAGS		= -T link.ld -n
LDFLAGS		= -m elf_i386 -e _start -Ttext 0x00001000

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
