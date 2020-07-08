# 文件路径
TEST 		= ./test
INFONES 	= ./infones
BOSH 		= ./bosh
LUA 		= ./lua

.PHONY: all clean

all: 
	$(MAKE) -C $(TEST)
	$(MAKE) -C $(INFONES)
	$(MAKE) -C $(BOSH)
	$(MAKE) -C $(LUA)
	
clean:
	$(MAKE) -C $(TEST) clean
	$(MAKE) -C $(INFONES) clean
	$(MAKE) -C $(BOSH) clean
	$(MAKE) -C $(LUA) clean