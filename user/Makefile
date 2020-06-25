# 文件路径
TEST 		= ./test
INFONES 	= ./infones
BOSH 		= ./bosh

.PHONY: all clean

all: 
	cd $(TEST) && make
	cd $(INFONES) && make
	cd $(BOSH) && make
	
clean:
	cd $(TEST) && make clean
	cd $(INFONES) && make clean
	cd $(BOSH) && make clean
	