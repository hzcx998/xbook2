X_LIBS		+= xlibc-0.0.1.a sgi-0.0.1.a pthread-0.0.1.a

NAME		:= test
SRC			+= main.c cond_sync.c

<<<<<<< HEAD
#The file path
DIR_LIBARY = ../../libary

# libarys
DIR_XLIBC 		= $(DIR_LIBARY)/xlibc
DIR_SGI 		= $(DIR_LIBARY)/sgi
DIR_PTHREAD 	= $(DIR_LIBARY)/pthread

#rom dir
DIR_ROM = ../../develop/rom

BIN = $(DIR_ROM)/bin/test
BIN_DIS = out.disasm

#flags
ASM_FLAGS	= -f elf \
			-I $(DIR_XLIBC)/include 

C_FLAGS		= -c -fno-builtin -Wall -Wunused -fno-PIE -m32 -fno-stack-protector
			
LDFLAGS		= -no-pie -m elf_i386 -e _start -Ttext 0x00001000


# dir
INCDIRS			=	./
INCDIRS			+=	$(DIR_XLIBC)/include
INCDIRS			+=	$(DIR_SGI)/include
INCDIRS			+=	$(DIR_PTHREAD)/include

SRCDIRS		+=	./

# 在每一个目录前面加一个-I 
X_INCDIRS	:=	$(patsubst %, -I %, $(INCDIRS))

# 替换成新的内容，存放到X_SRCDIRS中
X_SRCDIRS	:=	$(patsubst %, %, $(SRCDIRS))

# 找到C文件
X_CFILES	:=	$(foreach dir, $(X_SRCDIRS), $(wildcard $(dir)/*.c))

# 转换成对象文件。a.c -> .obj/a.o
X_COBJS		:=	$(X_CFILES:.c=.o)

# objects
OBJS			= 	$(X_COBJS)

# 被依赖库应放到后面。
LIB_FILE := $(DIR_SGI)/sgi-0.0.1.a \
			$(DIR_PTHREAD)/pthread-0.0.1.a \
			$(DIR_XLIBC)/xlibc-0.0.1.a \
			
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
=======
define CUSTOM_TARGET_CMD
echo [APP] $@; \
$(LD) $(X_LDFLAGS) $(X_OBJS) -o $@ $(patsubst %, -L%, $(X_LIBDIRS)) --start-group $(patsubst %, -l:%, $(X_LIBS)) --end-group; \
cp $@ $(srctree)/../develop/rom/bin
endef
>>>>>>> 17d36e39ee27fd9e4c6cc6e7237a014afddb6344
