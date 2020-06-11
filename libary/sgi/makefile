# tools
AR 				= ar
NASM 			= nasm
CC				= gcc
RM				= rm

# dir
DIR_INCLUDE 	= ./include
DIR_INCLUDE_XBOOK 	= ../xbooklib/include

# 
CORE_DIR 		= ./core
EVENT_DIR 		= ./event
WINMAN_DIR 		= ./winman

# flags
ARFLAGS 		= -cr
C_FLAGS			= 	-c -fno-builtin -nostdinc -Wall -Wunused -fno-PIE -m32 \
					-fno-stack-protector -std=c99 \
					-I $(DIR_INCLUDE) \
					-I $(DIR_INCLUDE_XBOOK) \
					

NASM_FLAGS		= 	-f elf \
					-I $(DIR_INCLUDE) \
					-I $(DIR_INCLUDE_XBOOK) \


CORE_OBJS	=		$(CORE_DIR)/display.o \
					$(CORE_DIR)/mem.o \
					$(CORE_DIR)/event.o \
					
WINMAN_OBJS	=		$(WINMAN_DIR)/window.o \

# objects
OBJS			= 	$(CORE_OBJS) $(WINMAN_OBJS)

# out file
LIBNAME = sgi-0.0.1.a

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
	