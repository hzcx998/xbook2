# 要执行的服务的路径
XLIBC 		= ./xlibc
PTHREAD 	= ./pthread
JPEG 		= ./jpeg
LIBPNG 		= ./libpng
ZLIB 		= ./zlib

.PHONY: all clean

all: 
	$(MAKE) -C $(XLIBC)
	$(MAKE) -C $(PTHREAD)
	$(MAKE) -C $(JPEG)
	$(MAKE) -C $(LIBPNG)
	$(MAKE) -C $(ZLIB)

clean: 
	$(MAKE) -C $(XLIBC) clean
	$(MAKE) -C $(PTHREAD) clean
	$(MAKE) -C $(JPEG) clean
	$(MAKE) -C $(LIBPNG) clean
	$(MAKE) -C $(ZLIB) clean

	