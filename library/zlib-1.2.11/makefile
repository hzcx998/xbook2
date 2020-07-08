# tools
AR 				= ar
NASM 			= nasm
CC				= gcc
RM				= rm

# flags
ARFLAGS 		= -cr
C_FLAGS			= 	-c -fno-builtin -nostdinc -Wall -Wunused -fno-PIE -m32 \
					-fno-stack-protector \

C_FLAGS			+=	-ffunction-sections -fdata-sections -ffreestanding -std=gnu99

NASM_FLAGS		= 	-f elf \
					
# 引入库头文件
DIR_XLIBC 		= ../xlibc/include

# dir
INCDIRS			=	./
INCDIRS			+=	$(DIR_XLIBC)

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

# out file
LIBNAME = zlib-1.2.11.a

# all need to do
all: $(LIBNAME) 

# make a libary file
$(LIBNAME): $(OBJS) 
	@echo [AR] making libary file $(LIBNAME)
	@$(AR) $(ARFLAGS) $(LIBNAME) $(OBJS)

%.o : %.c
	@echo [CC] $@ $<
	@$(CC) $(C_FLAGS) $(X_INCDIRS) -o $@ $<

%.o : %.asm
	@echo [NASM] $@ $<
	@$(NASM) $(NASM_FLAGS) $(X_INCDIRS) -o $@ $<

# clean .o .a files
clean: 
	-$(RM) $(LIBNAME)
	-$(RM) $(OBJS)
	