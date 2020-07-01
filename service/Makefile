# 要执行的服务的路径
FILESRV 	= ./filesrv
GUISRV 		= ./guisrv
INITSRV 	= ./initsrv
NETSRV 		= ./netsrv

.PHONY: all clean

all: 
	cd $(INITSRV) && make
	cd $(FILESRV) && make
	cd $(GUISRV) && make
	cd $(NETSRV) && make
	
clean: 
	cd $(INITSRV) && make clean
	cd $(FILESRV) && make clean
	cd $(GUISRV) && make clean
	cd $(NETSRV) && make clean
	