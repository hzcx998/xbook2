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
ARCH_WORDSIZE32_DIR 	= $(ARCH_DIR)/wordsize-32

ANSI_DIR 		= ./ansi
STDIO_DIR 		= ./stdio
SYSLIB_DIR 		= ./syslib
STDLIB_DIR 		= ./stdlib
PTHREAD_DIR 	= ./pthread
TIME_DIR 		= ./time
UNISTD_DIR		= ./unistd
MATH_DIR		= ./math
CTYPE_DIR		= ./ctype
SOCKET_DIR		= ./socket
ARPA_DIR		= ./arpa
STRING_DIR		= ./string
ENVIRON_DIR		= ./environ

# flags
ARFLAGS 		= -cr
C_FLAGS			= 	-c -fno-builtin -nostdinc -Wall -Wunused -fno-PIE -m32 \
					-fno-stack-protector -std=c99 \
					-I $(DIR_INCLUDE) \
					

NASM_FLAGS		= 	-f elf \
					-I $(DIR_INCLUDE) \
			

ARCH_X86_OBJS 	=	$(ARCH_X86_DIR)/xchg.o \
					$(ARCH_X86_DIR)/atomic.o \
					$(ARCH_X86_DIR)/_start.o

ARCH_WORDSIZE32_OBJS 	=	$(ARCH_WORDSIZE32_DIR)/divdi3.o \

ARCH_OBJS		= 	$(ARCH_X86_OBJS) $(ARCH_WORDSIZE32_OBJS)

ANSI_OBJS 		=	$(ANSI_DIR)/string.o

STDIO_OBJS 		= 	$(STDIO_DIR)/__stdio.o \
					$(STDIO_DIR)/__stdio_flush.o \
					$(STDIO_DIR)/__stdio_read.o \
					$(STDIO_DIR)/__stdio_write.o \
					$(STDIO_DIR)/asprintf.o \
					$(STDIO_DIR)/clearerr.o \
					$(STDIO_DIR)/fclose.o \
					$(STDIO_DIR)/feof.o \
					$(STDIO_DIR)/ferror.o \
					$(STDIO_DIR)/fflush.o \
					$(STDIO_DIR)/fgetc.o \
					$(STDIO_DIR)/fgetpos.o \
					$(STDIO_DIR)/fgets.o \
					$(STDIO_DIR)/fopen.o \
					$(STDIO_DIR)/fprintf.o \
					$(STDIO_DIR)/fputc.o \
					$(STDIO_DIR)/fputs.o \
					$(STDIO_DIR)/fread.o \
					$(STDIO_DIR)/freopen.o \
					$(STDIO_DIR)/fscanf.o \
					$(STDIO_DIR)/fseek.o \
					$(STDIO_DIR)/fsetpos.o \
					$(STDIO_DIR)/ftell.o \
					$(STDIO_DIR)/fwrite.o \
					$(STDIO_DIR)/getc.o \
					$(STDIO_DIR)/getchar.o \
					$(STDIO_DIR)/printf.o \
					$(STDIO_DIR)/putc.o \
					$(STDIO_DIR)/putchar.o \
					$(STDIO_DIR)/remove.o \
					$(STDIO_DIR)/rename.o \
					$(STDIO_DIR)/rewind.o \
					$(STDIO_DIR)/scanf.o \
					$(STDIO_DIR)/setbuf.o \
					$(STDIO_DIR)/setvbuf.o \
					$(STDIO_DIR)/snprintf.o \
					$(STDIO_DIR)/sprintf.o \
					$(STDIO_DIR)/sscanf.o \
					$(STDIO_DIR)/system.o \
					$(STDIO_DIR)/tmpfile.o \
					$(STDIO_DIR)/tmpnam.o \
					$(STDIO_DIR)/ungetc.o \
					$(STDIO_DIR)/vasprintf.o \
					$(STDIO_DIR)/vsnprintf.o \
					$(STDIO_DIR)/vsscanf.o

SYSLIB_OBJS 	= 	$(SYSLIB_DIR)/proc.o \
					$(SYSLIB_DIR)/res.o \
					$(SYSLIB_DIR)/syscall.o \
					$(SYSLIB_DIR)/time.o \
					$(SYSLIB_DIR)/trigger.o \
					$(SYSLIB_DIR)/vmm.o \
					$(SYSLIB_DIR)/brk.o \
					$(SYSLIB_DIR)/waitque.o \
					$(SYSLIB_DIR)/srvcall.o \
					$(SYSLIB_DIR)/auto.o \
					$(SYSLIB_DIR)/spinlock.o \
					$(SYSLIB_DIR)/fifo.o \
					
STDLIB_OBJS		= 	$(STDLIB_DIR)/atof.o \
					$(STDLIB_DIR)/atoi.o \
					$(STDLIB_DIR)/atol.o \
					$(STDLIB_DIR)/atoll.o \
					$(STDLIB_DIR)/rand.o \
					$(STDLIB_DIR)/abort.o \
					$(STDLIB_DIR)/stream.o \
					$(STDLIB_DIR)/errno.o \
					$(STDLIB_DIR)/setjmp.o \
					$(STDLIB_DIR)/longjmp.o \
					$(STDLIB_DIR)/qsort.o \
					$(STDLIB_DIR)/strntoimax.o \
					$(STDLIB_DIR)/strntoumax.o \
					$(STDLIB_DIR)/strtod.o \
					$(STDLIB_DIR)/strtof.o \
					$(STDLIB_DIR)/strtoimax.o \
					$(STDLIB_DIR)/strtol.o \
					$(STDLIB_DIR)/strtoll.o \
					$(STDLIB_DIR)/strtoull.o \
					$(STDLIB_DIR)/strtoumax.o \
					
TIME_OBJS		= 	$(TIME_DIR)/__secs_to_tm.o \
					$(TIME_DIR)/__tm_to_secs.o \
					$(TIME_DIR)/asctime.o \
					$(TIME_DIR)/clock.o \
					$(TIME_DIR)/ctime.o \
					$(TIME_DIR)/difftime.o \
					$(TIME_DIR)/gettimeofday.o \
					$(TIME_DIR)/gmtime.o \
					$(TIME_DIR)/localtime.o \
					$(TIME_DIR)/mktime.o \
					$(TIME_DIR)/strftime.o \
					$(TIME_DIR)/time.o \
				
PTHREAD_OBJS	=	$(PTHREAD_DIR)/pthread_entry.o \
					$(PTHREAD_DIR)/pthread.o \
					$(PTHREAD_DIR)/pthread_spinlock.o \
					$(PTHREAD_DIR)/pthread_mutex.o \
					$(PTHREAD_DIR)/pthread_cond.o \
					$(PTHREAD_DIR)/semaphore.o \

UNISTD_OBJS		= 	$(UNISTD_DIR)/file.o \
					$(UNISTD_DIR)/exec.o \
					$(UNISTD_DIR)/dir.o \
					$(UNISTD_DIR)/stat.o \
					$(UNISTD_DIR)/mount.o \

MATH_OBJS		= 	$(MATH_DIR)/acos.o \
					$(MATH_DIR)/asin.o \
					$(MATH_DIR)/atan.o \
					$(MATH_DIR)/atan2.o \
					$(MATH_DIR)/ceil.o \
					$(MATH_DIR)/cos.o \
					$(MATH_DIR)/cosh.o \
					$(MATH_DIR)/exp.o \
					$(MATH_DIR)/fabs.o \
					$(MATH_DIR)/floor.o \
					$(MATH_DIR)/fmod.o \
					$(MATH_DIR)/frexp.o \
					$(MATH_DIR)/log.o \
					$(MATH_DIR)/log2.o \
					$(MATH_DIR)/log10.o \
					$(MATH_DIR)/math.o \
					$(MATH_DIR)/modf.o \
					$(MATH_DIR)/pow.o \
					$(MATH_DIR)/round.o \
					$(MATH_DIR)/sin.o \
					$(MATH_DIR)/sinh.o \
					$(MATH_DIR)/sqrt.o \
					$(MATH_DIR)/tan.o \
					$(MATH_DIR)/tanh.o \
					
CTYPE_OBJS		= 	$(CTYPE_DIR)/ctype.o \

SOCKET_OBJS		= 	$(SOCKET_DIR)/socket.o \

ARPA_OBJS		= 	$(ARPA_DIR)/inet.o \

STRING_OBJS		= 	$(STRING_DIR)/memchr.o \
					$(STRING_DIR)/strdup.o \
					$(STRING_DIR)/strcasecmp.o \

ENVIRON_OBJS	= 	$(ENVIRON_DIR)/clearenv.o \
					$(ENVIRON_DIR)/environ.o \
					$(ENVIRON_DIR)/getenv.o \
					$(ENVIRON_DIR)/putenv.o \
					$(ENVIRON_DIR)/setenv.o \
					$(ENVIRON_DIR)/unsetenv.o \

PATH_OBJS		= 	path/basename.o \
					path/dirname.o \

CRYPTO_OBJS		= 	crypto/aes128.o \
					crypto/crc8.o \
					crypto/crc16.o \
					crypto/crc32.o \
					crypto/sha1.o \
					crypto/sha256.o

CHARSET_OBJS	= 	charset/charset.o \

LOCALE_OBJS		= 	locale/localeconv.o \
					locale/setlocale.o \

MALLOC_OBJS		= 	malloc/malloc2.o \

# objects
OBJS			= 	$(ARCH_OBJS) $(ANSI_OBJS) $(STDIO_OBJS) \
					$(SYSLIB_OBJS) $(STDLIB_OBJS) $(SOCKET_OBJS) \
					$(TIME_OBJS) $(UNISTD_OBJS) $(MATH_OBJS) $(CTYPE_OBJS) \
					$(ARPA_OBJS) $(STRING_OBJS) $(ENVIRON_OBJS) $(PATH_OBJS) \
					$(CRYPTO_OBJS) $(CHARSET_OBJS) $(LOCALE_OBJS) $(MALLOC_OBJS)

# out file
LIBNAME = xlibc-0.0.1.a
	
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
	