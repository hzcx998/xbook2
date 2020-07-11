# 要执行的服务的路径
XLIBC 		= ./xlibc
PTHREAD 	= ./pthread
JPEG 		= ./jpeg

.PHONY: all clean

all: 
	$(MAKE) -C $(XLIBC)
	$(MAKE) -C $(PTHREAD)
	$(MAKE) -C $(JPEG)

clean: 
	$(MAKE) -C $(XLIBC) clean
	$(MAKE) -C $(PTHREAD) clean
	$(MAKE) -C $(JPEG) clean
	