# tools
AR 			= ar
NASM 		= nasm
CC			= gcc
RM			= rm

# dir
DIR_INCLUDE 	= ./
FATFS_DIR		= ./

XBOOKLIB_INCLUDE 	= ../xbooklib/include

# flags
ARFLAGS := -cr
C_FLAGS	:=  -c -fno-builtin -nostdinc -Wall -Wunused -fno-PIE -m32 \
			-fno-stack-protector -std=c99 \
			-I $(DIR_INCLUDE) \
			-I $(XBOOKLIB_INCLUDE) \
			
NASM_FLAGS	:= -f elf \
			-I $(DIR_INCLUDE) \
			-I $(XBOOKLIB_INCLUDE) \

FATFS_OBJS = 	$(FATFS_DIR)diskio.o \
				$(FATFS_DIR)ff.o \
				$(FATFS_DIR)ffsystem.o \
				$(FATFS_DIR)ffunicode.o \
	
# objects
OBJS	:= 	$(FATFS_OBJS)

# out file
LIBNAME = fatfs-r0.14.a
	
# all need to do
all: $(LIBNAME) 

# make a library file
$(LIBNAME): $(OBJS) 
	@echo [AR] making library file $(LIBNAME)
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
	