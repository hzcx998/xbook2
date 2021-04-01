# System environment variable.
ifeq ($(OS),Windows_NT)
	HOSTOS		:= windows
else
	ifeq ($(shell uname),Darwin)
		HOSTOS		:= macos
	else
		ifeq ($(shell uname),Linux)
			HOSTOS		:= linux
		else
			HOSTOS		:= unix-like
		endif
	endif
endif

# tools
RM			= rm

# System environment variable.
ifeq ($(HOSTOS),windows)
	# !!! if you wanto complie this tools in windows, you need use gcc for windows
	# and config your own path. Below is my path.
	CC       = 	D:/Softwares/Mingw/bin/gcc.exe 		# need mingw gcc for windows
	# need mingw include and libs
	INCS     = 	-I"D:/Softwares/Mingw/include" \
				-DWIN32

	LIBS 	= 	-L"D:/Softwares/Mingw/lib" \
				-static-libgcc
	
	# out file
	BIN = ./fatfs.exe

else
		CC       	= gcc
		INCS     = -I /usr/include -I /usr/local/include
		LIBS     = -static-libgcc
		# out file
		BIN = ./fatfs
endif

# flags	
CFLAGS   = $(INCS) -g3

# objects		
OBJS = 	main.o \
		driver.o \
        diskio.o \
        ff.o \
        ffsystem.o \
        ffunicode.o \

.PHONY: all clean

# all need to do
all: $(BIN) 

ifeq ($(HOSTOS),macos)
$(BIN): $(OBJS)
	$(CC) $(OBJS) -o $(BIN)
else
$(BIN): $(OBJS)
	$(CC) $(OBJS) -o $(BIN) $(LIBS)
endif

%.o : %.c
	@echo [CC] $@ $<
	@$(CC) -c $(CFLAGS) -o $@ $<

# clean .o .a files
clean: 
	-$(RM) $(BIN)
	-$(RM) $(OBJS)
	
