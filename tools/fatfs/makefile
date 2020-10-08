# tools
RM			= rm

# System environment variable.
ifeq ($(OS),Windows_NT)
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

	ifeq ($(shell uname),Linux)
		CC       	= gcc
		INCS     = -I /usr/include -I /usr/local/include
		LIBS     = -static-libgcc
		# out file
		BIN = ./fatfs

	endif
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

$(BIN): $(OBJS)
	$(CC) $(OBJS) -o $(BIN) $(LIBS)

%.o : %.c
	@echo [CC] $@ $<
	@$(CC) -c $(CFLAGS) -o $@ $<

# clean .o .a files
clean: 
	-$(RM) $(BIN)
	-$(RM) $(OBJS)
	
