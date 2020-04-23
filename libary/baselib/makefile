# tools
AR 			= ar
NASM 		= nasm
CC			= gcc
RM			= rm

# dir
DIR_INCLUDE 	= ./include

ANSI_DIR = ./ansi
STDIO_DIR = ./stdio
SYSLIB_DIR = ./syslib

# flags
ARFLAGS := -cr
C_FLAGS	:=  -c -fno-builtin -nostdinc -Wall -Wunused -fno-PIE -m32 \
			-fno-stack-protector -std=c99 \
			-I $(DIR_INCLUDE)

NASM_FLAGS	:= -f elf \
			-I $(DIR_INCLUDE) \

ANSI_OBJS 	=	$(ANSI_DIR)/string.o

STDIO_OBJS 	= 	$(STDIO_DIR)/printf.o \
				$(STDIO_DIR)/vsprintf.o \

SYSLIB_OBJS = 	$(SYSLIB_DIR)/proc.o \
				$(SYSLIB_DIR)/res.o \
				$(SYSLIB_DIR)/syscall.o \
				$(SYSLIB_DIR)/time.o \
				$(SYSLIB_DIR)/trigger.o \
				$(SYSLIB_DIR)/vmm.o

# objects
OBJS	:= 	$(ANSI_OBJS) $(STDIO_OBJS) $(SYSLIB_OBJS)

# out file
LIBNAME = baselib.a
	
# all need to do
all: $(LIBNAME) 

# make a libary file
$(LIBNAME): $(OBJS) 
	@echo [AR] making libary file $(LIBNAME)
	@$(AR) $(ARFLAGS) $(LIBNAME) $(OBJS)

%.o : %.c
	@echo [CC] $@ $<
	@$(CC) $(C_FLAGS) -o $@ $<

%.o : %.asm
	@echo [NASM] $@ $<
	@$(NASM) $(NASM_FLAGS) -o $@ $<

# clean .o .a files
clean: 
	-$(RM) $(LIBNAME)
	-$(RM) $(OBJS)
	