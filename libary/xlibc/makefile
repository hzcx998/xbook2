# tools
AR 			= ar
NASM 		= nasm
CC			= gcc
RM			= rm

# dir
DIR_INCLUDE 	= ./include
DIR_CODE 		= 	./code

# depens lib: base lib
DIR_CODE_BASE_LIB 		= ../base/include

# flags
ARFLAGS := -r
C_FLAGS	:=  -fno-builtin -nostdinc -Wall -Wunused -fno-PIE -m32 \
			-fno-stack-protector -I $(DIR_INCLUDE) -I $(DIR_CODE_BASE_LIB) -c -std=c99
NASM_FLAGS	:= -I $(DIR_INCLUDE) -I $(DIR_CODE_BASE_LIB)

C_OBJS		:= 	$(DIR_CODE)/string.o \
				$(DIR_CODE)/vsprintf.o \
				$(DIR_CODE)/printf.o

# out file
LIBNAME = xlibc.a
			
# objects
OBJS = $(C_OBJS)

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
	