# 要执行的服务的路径
XLIBC 		= ./xlibc
PTHREAD 	= ./pthread
SGI 		= ./sgi

.PHONY: all clean

all: 
	cd $(XLIBC) && make
	cd $(PTHREAD) && make
	cd $(SGI) && make
	
clean: 
	cd $(XLIBC) && make clean
	cd $(PTHREAD) && make clean
	cd $(SGI) && make clean