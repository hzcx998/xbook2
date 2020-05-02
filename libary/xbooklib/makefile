# tools
AR 				= ar
NASM 			= nasm
CC				= gcc
RM				= rm

# dir
DIR_INCLUDE 	= ./include


# 
ARCH_DIR 		= ./arch
ARCH_X86_DIR 	= $(ARCH_DIR)/x86

ANSI_DIR 		= ./ansi
STDIO_DIR 		= ./stdio
SYSLIB_DIR 		= ./syslib
STDLIB_DIR 		= ./stdlib
PTHREAD_DIR 	= ./pthread
TIME_DIR 		= ./time
# flags
ARFLAGS 		= -cr
C_FLAGS			= 	-c -fno-builtin -nostdinc -Wall -Wunused -fno-PIE -m32 \
					-fno-stack-protector -std=c99 \
					-I $(DIR_INCLUDE)

NASM_FLAGS		= 	-f elf \
					-I $(DIR_INCLUDE) \

ARCH_X86_OBJS 	=	$(ARCH_X86_DIR)/xchg.o \
					$(ARCH_X86_DIR)/atomic.o

ARCH_OBJS		= 	$(ARCH_X86_OBJS)

ANSI_OBJS 		=	$(ANSI_DIR)/string.o

STDIO_OBJS 		= 	$(STDIO_DIR)/printf.o \
					$(STDIO_DIR)/vsprintf.o \

SYSLIB_OBJS 	= 	$(SYSLIB_DIR)/proc.o \
					$(SYSLIB_DIR)/res.o \
					$(SYSLIB_DIR)/syscall.o \
					$(SYSLIB_DIR)/time.o \
					$(SYSLIB_DIR)/trigger.o \
					$(SYSLIB_DIR)/vmm.o \
					$(SYSLIB_DIR)/brk.o \
					$(SYSLIB_DIR)/waitque.o

STDLIB_OBJS		= 	$(STDLIB_DIR)/rand.o \
					$(STDLIB_DIR)/abort.o \
					$(STDLIB_DIR)/stream.o \
					$(STDLIB_DIR)/malloc.o \
					$(STDLIB_DIR)/errno.o \

TIME_OBJS		= 	$(TIME_DIR)/time.o \
				
PTHREAD_OBJS	=	$(PTHREAD_DIR)/pthread_entry.o \
					$(PTHREAD_DIR)/pthread.o \
					$(PTHREAD_DIR)/pthread_spinlock.o \
					$(PTHREAD_DIR)/pthread_mutex.o \
					$(PTHREAD_DIR)/pthread_cond.o \
				
# objects
OBJS			= 	$(ARCH_OBJS) $(ANSI_OBJS) $(STDIO_OBJS) \
					$(SYSLIB_OBJS) $(STDLIB_OBJS) $(PTHREAD_OBJS) \
					$(TIME_OBJS)

# out file
LIBNAME = xbooklib-0.0.1.a
	
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
	