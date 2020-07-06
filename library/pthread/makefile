# tools
AR 				= ar
NASM 			= nasm
CC				= gcc
RM				= rm

# dir
DIR_INCLUDE 	= ./include
DIR_INCLUDE_XBOOK 	= ../xlibc/include

# flags
ARFLAGS 		= -cr
C_FLAGS			= 	-c -fno-builtin -nostdinc -Wall -Wunused -fno-PIE -m32 \
					-fno-stack-protector -std=gnu99 \
					-I $(DIR_INCLUDE) \
					-I $(DIR_INCLUDE_XBOOK) \
					
NASM_FLAGS		= 	-f elf \
					-I $(DIR_INCLUDE) \
					-I $(DIR_INCLUDE_XBOOK) \


PTHREAD_OBJS	=	pthread_entry.o \
					pthread.o \
					pthread_spinlock.o \
					pthread_mutex.o \
					pthread_cond.o \
					semaphore.o \

# objects
OBJS			= 	$(PTHREAD_OBJS)

# out file
LIBNAME = pthread-0.0.1.a

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
	