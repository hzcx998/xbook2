# 要执行的服务的路径
XLIBC 		= ./xlibc
PTHREAD 	= ./pthread
SGI 		= ./sgi
FREETYPE	= ./freetype-2.10.1
JPEG 		= ./jpeg-9d
LIBPNG 		= ./libpng-1.6.37
ZLIB 		= ./zlib-1.2.11

.PHONY: all clean

all: 
	$(MAKE) -C $(XLIBC)
	$(MAKE) -C $(PTHREAD)
	$(MAKE) -C $(SGI)
	$(MAKE) -C $(FREETYPE)
	$(MAKE) -C $(JPEG)
	$(MAKE) -C $(LIBPNG)
	$(MAKE) -C $(ZLIB)
	
clean: 
	$(MAKE) -C $(XLIBC) clean
	$(MAKE) -C $(PTHREAD) clean
	$(MAKE) -C $(SGI) clean
	$(MAKE) -C $(FREETYPE) clean
	$(MAKE) -C $(JPEG) clean
	$(MAKE) -C $(LIBPNG) clean
	$(MAKE) -C $(ZLIB) clean
	