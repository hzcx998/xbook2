# tools
AR 			= ar
NASM 		= nasm
CC			= gcc
RM			= rm

# dir
DIR_INCLUDE 	= ./include
# other lib
DIR_STDLIB 		= ../stdlib/include


# flags
ARFLAGS := -r
C_FLAGS	:=  -c -fno-builtin -nostdinc -Wall -Wunused -fno-PIE -m32 \
			-fno-stack-protector  -std=c99 \
			-I $(DIR_INCLUDE) \
			-I $(DIR_STDLIB)
			
NASM_FLAGS	:= -f elf \
			-I $(DIR_INCLUDE) \
			-I $(DIR_STDLIB)

OBJS	:=	syscall.o \
			proc.o \
			vmm.o \
			res.o \
			time.o \

# out file
LIBNAME = xcore.a

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
	