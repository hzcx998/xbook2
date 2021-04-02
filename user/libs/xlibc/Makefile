X_ASFLAGS	:= -f elf -I xlibc/include
X_INCDIRS	:= xlibc/include

SRC			:= arch/x86/*.asm arch/x86/*.c arch/wordsize-32/*.c 
SRC			+= ansi/*.c arpa/*.c charset/*.c crypto/*.c ctype/*.c
SRC			+= environ/*.c locale/*.c malloc/*.c math/*.c stdio/*.c
SRC			+= stdlib/*.c string/*.c syslib/*.c time/*.c unistd/*.c
SRC			+= syslib/syscall.asm
SRC			+= gcc/*.c termios/*.c

TARGET_TYPE	:= static
NAME		:= xlibc
