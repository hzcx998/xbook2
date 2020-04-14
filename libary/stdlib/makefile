# tools
AR 			= ar
NASM 		= nasm
CC			= gcc
RM			= rm

# dir
DIR_INCLUDE 	= ./include
# other lib
DIR_XCORE 	= ../xcore/include

# flags
ARFLAGS := -cr
C_FLAGS	:=  -c -fno-builtin -nostdinc -Wall -Wunused -fno-PIE -m32 \
			-fno-stack-protector -std=c99 \
			-I $(DIR_INCLUDE) \
			-I $(DIR_XCORE) 
NASM_FLAGS	:= -f elf \
			-I $(DIR_INCLUDE) \
			-I $(DIR_XCORE)
							
# objects
OBJS	:= 	string.o \
			vsprintf.o \
			printf.o \
			trigger.o \

# out file
LIBNAME = stdlib.a
	
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
	