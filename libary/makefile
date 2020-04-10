# tools
AR 			= ar
NASM 		= nasm
CC			= gcc
RM			= rm

# dir
DIR_INCLUDE 	= ./include
DIR_XBOOK 		= 	./xbook
DIR_XCORE 		= 	./xcore
DIR_STDLIB 		= 	./stdlib

# flags
ARFLAGS := -r
C_FLAGS	:=  -c -fno-builtin -nostdinc -Wall -Wunused -fno-PIE -m32 \
			-fno-stack-protector -I $(DIR_INCLUDE) -std=c99
NASM_FLAGS	:= -I $(DIR_INCLUDE) -f elf 

XBOOK_OBJS	:=	$(DIR_XBOOK)/usrmsg_x86.o \
				$(DIR_XBOOK)/xbook.o \
				
XCORE_OBJS	:=	$(DIR_XCORE)/syscall.o \
				$(DIR_XCORE)/proc.o \
				$(DIR_XCORE)/vmm.o \
				$(DIR_XCORE)/res.o \

STDLIB_OBJS	:= 	$(DIR_STDLIB)/string.o \
				$(DIR_STDLIB)/vsprintf.o \
				$(DIR_STDLIB)/printf.o \
				
# out file
LIBNAME = xlib.a
			
# objects
OBJS = $(XCORE_OBJS) $(STDLIB_OBJS)

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
	