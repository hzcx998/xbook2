# tools
AR 			= ar
NASM 		= nasm
CC			= gcc
RM			= rm

# dir
DIR_INCLUDE 	= ./include
DIR_CODE 		= 	./code

# flags
ARFLAGS := -r
C_FLAGS	:=  -fno-builtin -nostdinc -Wall -Wunused -fno-PIE -m32 \
			-fno-stack-protector -I $(DIR_INCLUDE) -c -std=c99
NASM_FLAGS	:= -I $(DIR_INCLUDE)

# arch 
ARCH_X86 := usrmsg_x86.o
ARCH_X64 := usrmsg_x64.o

ARCH_OBJS	:=  $(DIR_CODE)/$(ARCH_X86)

C_OBJS		:= $(DIR_CODE)/xbook.o

# out file
LIBNAME = baselib.a
			
# objects
OBJS = $(ARCH_OBJS) $(C_OBJS)

# all need to do
all: $(LIBNAME) 

# make a libary file
$(LIBNAME): $(OBJS) 
	@echo [AR] making libary file $(LIBNAME)
	@$(AR) $(ARFLAGS) $(LIBNAME) $(OBJS)

%.o : %.c
	@echo [CC] $@ $<
	@$(CC) $(C_FLAGS) -c -o $@ $<

%.o : %.asm
	@echo [NASM] $@ $<
	@$(NASM) $(NASM_FLAGS) -f elf -o $@ $<

# clean .o .a files
clean: 
	-$(RM) $(LIBNAME)
	-$(RM) $(OBJS)
	