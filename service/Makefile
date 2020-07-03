# 要执行的服务的路径
FILESRV 	= ./filesrv
GUISRV 		= ./guisrv
INITSRV 	= ./initsrv
NETSRV 		= ./netsrv

.PHONY: all clean

all: 
	$(MAKE) -C $(INITSRV)
	$(MAKE) -C $(FILESRV)
	$(MAKE) -C $(GUISRV)
	$(MAKE) -C $(NETSRV)
	
clean: 
	$(MAKE) -C $(INITSRV) clean
	$(MAKE) -C $(FILESRV) clean
	$(MAKE) -C $(GUISRV) clean
	$(MAKE) -C $(NETSRV) clean
	