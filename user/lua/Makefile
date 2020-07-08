# makefile for test

#The tools name
NASM 		= nasm
CC			= gcc
LD 			= ld
DD 			= dd
OBJDUMP		= objdump
COPY		= cp

#The file path
DIR_LIBARY = ../../libary

# libarys
DIR_XLIBC 		= $(DIR_LIBARY)/xlibc

#rom dir
DIR_ROM = ../../develop/rom

BIN = $(DIR_ROM)/bin/lua
BIN_DIS = out.disasm

#flags
ASM_FLAGS	= -f elf \
			-I $(DIR_XLIBC)/include 

C_FLAGS		= -c -fno-builtin -Wall -Wunused -fno-PIE -m32 -fno-stack-protector
			
LDFLAGS		= -no-pie -m elf_i386 -e _start -Ttext 0x00001000


# dir
INCDIRS			=	./lua-5.4.0
INCDIRS			+=	$(DIR_XLIBC)/include

SRCDIRS		+=	./
SRCDIRS		+=	./lua-5.4.0

# 在每一个目录前面加一个-I 
X_INCDIRS	:=	$(patsubst %, -I %, $(INCDIRS))

# 替换成新的内容，存放到X_SRCDIRS中
X_SRCDIRS	:=	$(patsubst %, %, $(SRCDIRS))

# 找到C文件
X_CFILES	:=	$(foreach dir, $(X_SRCDIRS), $(wildcard $(dir)/*.c))

# 转换成对象文件。a.c -> .obj/a.o
X_COBJS		:=	$(X_CFILES:.c=.o)

# objects
OBJS		= 	$(X_COBJS)

# 被依赖库应放到后面。
LIB_FILE := $(DIR_XLIBC)/xlibc-0.0.1.a \
			
#First read here
.PHONY: all dis

all: compile link

#Compile files
compile: $(OBJS)

link: $(BIN)
$(BIN): $(OBJS) $(LIB_FILE)
	$(LD) $(LDFLAGS) -o $(BIN) $(OBJS) $(LIB_FILE)


dis: $(BIN)
	$(OBJDUMP) -M intel -D $(BIN) > $(BIN_DIS)

#Clean temporary files
clean:
	-rm *.o
	-rm $(BIN)

%.o: %.asm
	$(NASM) $(ASM_FLAGS) -o $@ $<
	
%.o: %.c
	$(CC) $(C_FLAGS) $(X_INCDIRS) -c -o $@ $<
