# 要执行的服务的路径
XLIBC 		= ./xlibc
PTHREAD 	= ./pthread
SGI 		= ./sgi

.PHONY: all clean

all: 
	$(MAKE) -C $(XLIBC)
	$(MAKE) -C $(PTHREAD)
	$(MAKE) -C $(SGI)
	
clean: 
	$(MAKE) -C $(XLIBC) clean
	$(MAKE) -C $(PTHREAD) clean
	$(MAKE) -C $(SGI) clean